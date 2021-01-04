#include "main.h"
#include "enqu.h"
#include "enqu_x265.h"

namespace enqu {

/* input & output */
format g_f, g_of;
int g_nf = 0;

/* slider (io) */
int g_si = 0;

/**/
std::vector<int> g_sof;

const VSAPI* vsapi = 0;
VSScript* se = 0;
VSCore* core = 0;

char error_msg[1024];

std::map<std::string, int> format::name2id;
std::map<int, std::string> format::id2name;

std::unique_ptr<video_buf_map> g_buf;
std::unique_ptr<layout> g_layout;
QSlider* g_slider;
QGraphicsView* g_view;
QGraphicsPixmapItem* g_pixmap;
QLabel* g_stats;

format::format(int id, int h, int w)
	: id(id), h(h), w(w)
{
	const VSFormat* f = vsapi->getFormatPreset(id, core);
	bit_depth = f->bitsPerSample;
	np = f->numPlanes;
	ssx = f->subSamplingH;
}

format::format(int h, int w, int bit_depth, int np, int ssx)
	: h(h), w(w), bit_depth(bit_depth), np(np), ssx(ssx)
{
	const VSFormat* f = vsapi->registerFormat(np == 1 ? VSColorFamily::cmGray : VSColorFamily::cmYUV, stInteger, bit_depth, ssx, ssx, core);
	id = f->id;
}

struct video_buf
{
	std::vector<uint8_t*> inf;
	VSNodeRef* node;
	std::unique_ptr<uint8_t[]> buf;
	uint8_t* ptr;
	format f;
	int out_w, out_h;
	video_buf(const format& f, VSNodeRef* node_)
		: f(f)
		, out_w(g_of.w)
		, out_h(g_of.h)
	{
		if (!node_)
			throw "";
		size_t size = f.frame_size() * g_nf;
		ptr = new uint8_t[size];
		if (!ptr)
			sprintf(error_msg, ""), throw error_msg;
		buf.reset(ptr);
		node = invoke_raws_to_out(f, &ptr, out_h, out_w);
		if (!node)
			throw "";
		inf.resize(g_nf);
		for (int n = 0; n < g_nf; n++)
			inf[n] = ptr, node_get_frame(n, node_, &ptr);
		vsapi->freeNode(node_);
	}
	~video_buf()
	{
		vsapi->freeNode(node);
	}
	void out(int h, int w, uint8_t* src, uint8_t* out)
	{
		ptr = src;
		if (h != out_h || w != out_w)
		{
			out_w = w, out_h = h;
			vsapi->freeNode(node);
			node = invoke_raws_to_out(f, &ptr, h, w);
			if (!node)
				throw "";
		}
		node_get_frame(0, node, &out);
	}
	void out(int n, int h, int w, uint8_t* dst)
	{
		out(h, w, inf[n], dst);
	}
	operator uint8_t** () { return inf.data(); }
};

struct video_buf_map_impl : video_buf_map
{
	std::map<int, std::unique_ptr<video_buf>> map;
	VSNodeRef* node; // input raws to node
	std::unique_ptr<uint8_t[]> buf;
	uint8_t* ptr;
	video_buf* input;
	int out_w, out_h;
	video_buf* at(const format& f)
	{
		if (!map.count(f.id))
			map[f.id] = std::make_unique<video_buf>(f, invoke_node_to_src(f, node));
		return map.at(f.id).get();
	}
	video_buf_map_impl(VSNodeRef* node_)
		: out_w(g_of.w)
		, out_h(g_of.h)
	{
		ptr = new uint8_t[(size_t)out_h * out_w * 4];
		buf.reset(ptr);
		input = new video_buf(g_f, node_);
		map.emplace(g_f.id, input);
		node = invoke_raws_to_node(*input);
	}
	~video_buf_map_impl()
	{
		vsapi->freeNode(node);
	}
	uint8_t** src(const format& f)
	{
		return *at(f);
	}
	uint8_t* out(int n, int h, int w)
	{
		if (h != out_h || w != out_w)
		{
			ptr = new uint8_t[(size_t)h * w * 4];
			buf.reset(ptr);
			out_w = w, out_h = h;
		}
		input->out(n, h, w, ptr);
		return ptr;
	}
	uint8_t* out(int h, int w, uint8_t* src, const format& f)
	{
		if (h != out_h || w != out_w)
		{
			ptr = new uint8_t[(size_t)h * w * 4];
			buf.reset(ptr);
			out_w = w, out_h = h;
		}
		at(f)->out(h, w, src, ptr);
		return ptr;
	}
};

// to input node
VSNodeRef* invoke_raws_to_node(uint8_t** ptr)
{
	VSPlugin* vp_p = vsapi->getPluginById("xxx.xyz.vp", core);
	if (!vp_p)
		return 0;
	VSMap* args, * res;
	args = vsapi->createMap();
	vsapi->propSetInt(args, "ptr", (intptr_t)ptr, paReplace);
	vsapi->propSetInt(args, "width", g_f.w, paReplace);
	vsapi->propSetInt(args, "height", g_f.h, paReplace);
	vsapi->propSetInt(args, "format_id", g_f.id, paReplace);
	vsapi->propSetInt(args, "num_frames", g_nf, paReplace);
	res = vsapi->invoke(vp_p, "raws", args);
	vsapi->freeMap(args);
	VSNodeRef* node;
	if (const char* err = vsapi->getError(res); !err)
		node = vsapi->propGetNode(res, "clip", 0, 0);
	else
		node = 0;
	vsapi->freeMap(res);
	return node;
}

VSNodeRef* invoke_node_to_src(const format& f, VSNodeRef* node)
{
	VSPlugin* resize_p = vsapi->getPluginById("com.vapoursynth.resize", core);
	if (!resize_p)
		return 0;
	VSMap* args, * res;
	args = vsapi->createMap();
	vsapi->propSetInt(args, "width", f.w, paReplace);
	vsapi->propSetInt(args, "height", f.h, paReplace);
	vsapi->propSetInt(args, "format", f.id, paReplace);
	vsapi->propSetData(args, "dither_type", "error_diffusion", 15, paReplace);
	vsapi->propSetNode(args, "clip", node, paReplace);
	res = vsapi->invoke(resize_p, "Spline36", args);
	vsapi->freeMap(args);
	if (const char* err = vsapi->getError(res); !err)
		node = vsapi->propGetNode(res, "clip", 0, 0);
	else
		node = 0;
	vsapi->freeMap(res);
	return node;
}

VSNodeRef* invoke_raws_to_out(const format& f, uint8_t** ptr, int h, int w)
{
	VSPlugin* vp_p = vsapi->getPluginById("xxx.xyz.vp", core),
		* resize_p = vsapi->getPluginById("com.vapoursynth.resize", core);
	VSPlugin* std_p = vsapi->getPluginById("com.vapoursynth.std", core);
	if (!vp_p || !resize_p)
		return 0;
	VSMap* args, * res;
	args = vsapi->createMap();
	vsapi->propSetInt(args, "ptr", (intptr_t)ptr, paReplace);
	vsapi->propSetInt(args, "width", f.w, paReplace);
	vsapi->propSetInt(args, "height", f.h, paReplace);
	vsapi->propSetInt(args, "format_id", f.id, paReplace);
	vsapi->propSetInt(args, "num_frames", g_nf, paReplace);
	res = vsapi->invoke(vp_p, "raws", args);
	vsapi->freeMap(args);
	VSNodeRef* node;
	if (const char* err = vsapi->getError(res); !err)
		node = vsapi->propGetNode(res, "clip", 0, 0);
	else
		node = 0;
	vsapi->freeMap(res);
	if (!node)
		return 0;
	args = vsapi->createMap();
	vsapi->propSetInt(args, "width", w, paReplace);
	vsapi->propSetInt(args, "height", h, paReplace);
	vsapi->propSetInt(args, "format", pfCompatBGR32, paReplace);
	vsapi->propSetData(args, "matrix_in_s", "709", 3, paReplace);
	vsapi->propSetData(args, "dither_type", "error_diffusion", 15, paReplace);
	vsapi->propSetNode(args, "clip", node, paReplace);
	vsapi->freeNode(node);
	res = vsapi->invoke(resize_p, "Spline36", args);
	vsapi->freeMap(args);
	if (const char* err = vsapi->getError(res); !err)
		node = vsapi->propGetNode(res, "clip", 0, 0);
	else
		node = 0;
	vsapi->freeMap(res);
	args = vsapi->createMap();
	vsapi->propSetNode(args, "clip", node, paReplace);
	vsapi->freeNode(node);
	res = vsapi->invoke(std_p, "FlipVertical", args);
	vsapi->freeMap(args);
	if (const char* err = vsapi->getError(res); !err)
		node = vsapi->propGetNode(res, "clip", 0, 0);
	else
		node = 0;
	vsapi->freeMap(res);
	return node;
}

int node_get_frame(int n, VSNodeRef* node, uint8_t** ptr)
{
	const VSFrameRef* f = vsapi->getFrame(n, node, 0, 0);
	if (!f)
		return -1;
	const VSFormat* ff = vsapi->getFrameFormat(f);
	for (int p = 0, np = ff->numPlanes; p < np; p++)
	{
		int h = vsapi->getFrameHeight(f, p), w = vsapi->getFrameWidth(f, p),
			stride = vsapi->getStride(f, p), rowsize = w * ff->bytesPerSample;
		const uint8_t* psrc = vsapi->getReadPtr(f, p);
		if (rowsize != stride)
		{
			for (size_t i = 0; i < h; i++)
				*ptr = std::copy_n(psrc + i * stride, rowsize, *ptr);
		}
		else
			*ptr = std::copy_n(psrc, h * rowsize, *ptr);
	}
	vsapi->freeFrame(f);
	return 0;
}

void res::resize(int id, std::pair<int, int> size, size_t of_count)
{
	f = format(id, size.second, size.first);
	if (!buf.empty())
		free(buf[0]);
	size_t frame_size = f.frame_size(), bytes = frame_size * of_count;
	uint8_t* ptr = (uint8_t*)malloc(bytes);
	if (!ptr)
		return;
	buf.resize(of_count);
	for (int n = 0; n < of_count; n++)
	{
		buf[n] = ptr;
		ptr += frame_size;
	}
}

void close_input()
{
	g_slider->setMaximum(0);
	if (g_buf)
		g_pixmap->setPixmap(QPixmap());
	g_sof.clear();
	g_buf.reset();
}

void out_changed()
{
	g_view->setSceneRect(0, 0, g_of.w, g_of.h);
	if (g_of.w > 1280 || g_of.h > 720)
		g_view->setDragMode(QGraphicsView::ScrollHandDrag);
	else
		g_view->setDragMode(QGraphicsView::NoDrag);
}

int pixmap_update(int si)
{
	if (!g_buf)
		return -1;
	int w = g_of.w, h = g_of.h;
	g_pixmap->setPixmap(QPixmap::fromImage(QImage((const uchar*)g_buf->out(si, h, w), w, h, QImage::Format_RGB32)));
	return 0;
}

void open()
{
	QString ret = QFileDialog::getOpenFileName(0, QObject::tr(""), QObject::tr(""), QObject::tr("(*.vpy *.mkv);;(*)"), 0, 0);
	if (ret.isEmpty())
		return;
	close_input();
	std::string path = ret.toStdString();
	VSNodeRef* node = 0;
	do
	{
		if (path.ends_with(".vpy"))
		{
			if (vsscript_evaluateFile(&se, path.c_str(), efSetWorkingDir))
				break;
			node = vsscript_getOutput(se, 0);
		}
		if (!node)
			break;
		const VSVideoInfo* vi = vsapi->getVideoInfo(node);
		g_f.id = vi->format->id;
		g_f.bit_depth = vi->format->bitsPerSample;
		g_f.np = vi->format->numPlanes;
		g_f.ssx = vi->format->subSamplingH;
		if (vi->format->subSamplingW != g_f.ssx)
			break;
		g_f.h = vi->height;
		g_f.w = vi->width;
		g_nf = vi->numFrames;
		g_of = g_f;
		try
		{
			g_buf.reset(new video_buf_map_impl(node));
		}
		catch (const char* error)
		{
			break;
		}
		for (int i = 0; i < g_nf; i++)
			g_sof.push_back(i);
		g_slider->setMaximum(g_sof.size() - 1);
		if (g_layout)
			g_layout->input_changed();
		out_changed();
		g_si = std::clamp(g_si, 0, g_nf);
		pixmap_update(g_si);
		return;
	} while (0);
	if (node)
		vsapi->freeNode(node);
}

main_window::main_window()
{
	try
	{
		if (!vsscript_init())
			throw "!vsscript_init()";
		vsapi = vsscript_getVSApi();
		if (!vsapi)
			throw "!vsapi";
		if (vsscript_createScript(&se))
			sprintf(error_msg, "%s", vsscript_getError(se)), throw error_msg;
		core = vsscript_getCore(se);
		if (!core)
			throw "!core";
		vsapi->setThreadCount(1, core);
	}
	catch (const char* msg)
	{
		if (vsapi)
		{
			if (se)
				vsscript_freeScript(se), se = 0;
			vsapi = 0;
			vsscript_finalize();
		}
		QMessageBox::warning(this, QObject::tr(""), msg);
	}
	resize(1800, 900);
	QWidget* center = new QWidget;
	setCentralWidget(center);
	QVBoxLayout* box = new QVBoxLayout(center);
	QTabWidget* tab = new QTabWidget;
	QMenu* menu0 = menuBar()->addMenu(tr("&File"));
	menu0->addAction(tr("..."), [] { open(); });
	menu0->addSeparator();
	QMenu* menu1 = menuBar()->addMenu(tr("&Tools"));
	QActionGroup* group = new QActionGroup(this);
	group->setExclusive(true);
	{
		QAction* _;
		_ = menu1->addAction(tr("x265"), this, [=]
		{
			if (g_layout)
				g_layout.reset();
			g_layout = make_x265_layout(tab);
		});
		_->setCheckable(true);
		group->addAction(_);
	}
	menu1->addSeparator();
	g_slider = new QSlider(Qt::Orientation::Horizontal);
	box->addWidget(g_slider);
	g_slider->setTickPosition(QSlider::TickPosition::TicksBothSides);
	g_slider->setFixedWidth(100);
	g_slider->setRange(0, 0);
	connect(g_slider, &QSlider::valueChanged,
		[=](int n)
	{
		g_si = n;
		if (g_layout)
			g_layout->pixmap_update(n);
		else
			pixmap_update(n);
	});
	box->addWidget(tab);
	QDockWidget* view_dock = new QDockWidget;
	QGraphicsScene* scene = new QGraphicsScene(QRect(0, 0, 1280, 720));
	g_pixmap = scene->addPixmap(QPixmap());
	g_view = new QGraphicsView;
	g_view->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	g_view->setInteractive(false);
	g_view->setScene(scene);
	view_dock->setWidget(g_view);
	addDockWidget(Qt::RightDockWidgetArea, view_dock);
	QDockWidget* stat_dock = new QDockWidget;
	g_stats = new QLabel;
	stat_dock->setWidget(g_stats);
	addDockWidget(Qt::RightDockWidgetArea, stat_dock);
	installEventFilter(this);
}

main_window::~main_window()
{
	g_layout.reset();
	g_buf.reset();
	if (vsapi)
	{
		if (se)
			vsscript_freeScript(se);
		vsscript_finalize();
	}
}

bool main_window::eventFilter(QObject* obj, QEvent* event)
{
	switch (event->type())
	{
	case QEvent::KeyPress:
	{
		QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
		int key = key_event->key(), modifiers = key_event->modifiers();
		switch (key)
		{
		case Qt::Key_Q:
			QApplication::quit();
			return 1;
		case Qt::Key_Plus:
			g_of.w = std::clamp(g_of.w * 2, 1280, g_f.w * 4);
			g_of.h = std::clamp(g_of.h * 2, 720, g_f.h * 4);
			out_changed();
			return 1;
		case Qt::Key_Minus:
			g_of.w = std::clamp(g_of.w / 2, 1280, g_f.w * 4);
			g_of.h = std::clamp(g_of.h / 2, 720, g_f.h * 4);
			out_changed();
			return 1;
		}
	}
	}
	if (g_layout && g_layout->event(obj, event))
		return 1;
	return QObject::eventFilter(obj, event);
}

template< typename T>
std::string p2str(const std::any& x)
{
	char buf[40] = {};
	if constexpr (std::is_integral_v<T>)
		sprintf(buf, "%d", (int)std::any_cast<T>(x));
	else if constexpr (std::is_floating_point_v<T>)
		sprintf(buf, "%.2f", (float)std::any_cast<T>(x));
	else if constexpr (std::is_same_v<T, std::tuple<int, int, int>>)
	{
		T y = std::any_cast<T>(x);
		sprintf(buf, "%d,%d,%d", std::get<0>(y), std::get<1>(y), std::get<2>(y));
	}
	return std::string(buf);
}

template std::string p2str<int>(const std::any& x);
template std::string p2str<float>(const std::any& x);
template std::string p2str<std::tuple<int, int, int>>(const std::any& x);

template< typename T>
int str2p(const char** str, void* x)
{
	int n = 0;
	if constexpr (std::is_same_v<T, bool>)
		n = sscanf(*str, "%d\n%n", (T*)x, &n) == 1 ? n : 0;
	else if constexpr (std::is_same_v<T, int>)
		n = sscanf(*str, "%d\n%n", (T*)x, &n) == 1 ? n : 0;
	else if constexpr (std::is_same_v<T, float>)
		n = sscanf(*str, "%f\n%n", (T*)x, &n) == 1 ? n : 0;
	else if constexpr (std::is_same_v<T, std::tuple<int, int, int>>)
	{
		T& y = *(T*)(x);
		n = sscanf(*str, "%d,%d,%d\n%n", &std::get<0>(y), &std::get<1>(y), &std::get<2>(y), &n) == 3 ? n : 0;
	}
	*str += n; return n;
}

template int str2p<int>(const char** str, void* x);
template int str2p<float>(const char** str, void* x);
template int str2p<std::tuple<int, int, int>>(const char** str, void* x);

template<>
std::string p2str<format>(const std::any& _x)
{
	int x = std::any_cast<int>(_x);
	if (!format::id2name.count(x) && format::try_get_format_preset(x))
		throw;
	return format::id2name.at(x);
}

template<>
int str2p<format>(const char** str, void* _x)
{
	int* x = (int*)_x;
	int n = 0;
	char buf[16] = { 0 };
	n = sscanf(*str, "%15[^\n]\n%n", buf, &n) == 1 ? n : 0;//...
	std::string name = buf;
	if (!format::name2id.count(name))
	{
		// construct from name
		return 0;
	}
	*x = format::name2id.at(buf);
	*str += n; return n;
}

template<>
std::string p2str< std::array<float, 2>>(const std::any& x)
{
	char buf[40] = {};
	auto a = std::any_cast<std::array<float, 2>>(x);
	float* y = a.data();
	sprintf(buf, "%.2f,%.2f", y[0], y[1]);
	return std::string(buf);
}

template<>
int str2p< std::array<float, 2>>(const char** str, void* x)
{
	int n = 0;
	float* y = ((std::array<float, 2>*)(x))->data();
	n = sscanf(*str, "%f,%f\n%n", y, y + 1, &n) == 2 ? n : 0;
	*str += n;
	return n;
}

template<>
std::string p2str< std::array<int, 2>>(const std::any& x)
{
	char buf[40] = {};
	auto a = std::any_cast<std::array<int, 2>>(x);
	int* y = a.data();
	sprintf(buf, "%d,%d", y[0], y[1]);
	return std::string(buf);
}

template<>
int str2p< std::array<int, 2>>(const char** str, void* x)
{
	int n = 0;
	int* y = ((std::array<int, 2>*)(x))->data();
	n = sscanf(*str, "%d,%d\n%n", y, y + 1, &n) == 2 ? n : 0;
	*str += n;
	return n;
}

template< typename T>
void plane_copy(size_t h, size_t w, uint8_t** ppdst, uint8_t* psrc, size_t src_stride)
{
	size_t dst_stride = w * sizeof(T);
	for (size_t i = 0; i < h; i++)
	{
		for (size_t j = 0; j < w; j++)
		{
			T x = reinterpret_cast<T*>(psrc + i * src_stride)[j];
			reinterpret_cast<T*>(*ppdst + i * dst_stride)[j] = x;
		}
	}
	*ppdst = *ppdst + dst_stride * h;
}

template< typename T>
void copy(size_t h, size_t w, size_t np, size_t ssh, size_t ssw, uint8_t* pdst, uint8_t** ppsrc, int* src_stride)
{
	plane_copy<T>(h, w, &pdst, ppsrc[0], src_stride[0]);
	for (size_t p = 1; p < np; p++)
		plane_copy<T>(h >> ssh, w >> ssw, &pdst, ppsrc[p], src_stride[p]);
}

template void copy<uint8_t>(size_t h, size_t w, size_t np, size_t ssh, size_t ssw, uint8_t* pdst, uint8_t** ppsrc, int* src_stride);
template void copy<uint16_t>(size_t h, size_t w, size_t np, size_t ssh, size_t ssw, uint8_t* pdst, uint8_t** ppsrc, int* src_stride);

template< typename T>
void plane_copy_s(size_t h, size_t w, uint8_t** ppsrc, uint8_t* pdst, size_t dst_stride)
{
	size_t src_stride = w * sizeof(T);
	for (size_t i = 0; i < h; i++)
	{
		for (size_t j = 0; j < w; j++)
		{
			T x = reinterpret_cast<T*>(*ppsrc + i * src_stride)[j];
			reinterpret_cast<T*>(pdst + i * dst_stride)[j] = x;
		}
	}
	*ppsrc = *ppsrc + src_stride * h;
}

template< typename T>
void copy_s(size_t h, size_t w, size_t np, size_t ssh, size_t ssw, uint8_t* psrc, uint8_t** ppdst, int* dst_stride)
{
	plane_copy_s<T>(h, w, &psrc, ppdst[0], dst_stride[0]);
	for (size_t p = 1; p < np; p++)
		plane_copy_s<T>(h >> ssh, w >> ssw, &psrc, ppdst[p], dst_stride[p]);
}

template void copy_s<uint8_t>(size_t h, size_t w, size_t np, size_t ssh, size_t ssw, uint8_t* psrc, uint8_t** ppdst, int* dst_stride);
template void copy_s<uint16_t>(size_t h, size_t w, size_t np, size_t ssh, size_t ssw, uint8_t* psrc, uint8_t** ppdst, int* dst_stride);

}
