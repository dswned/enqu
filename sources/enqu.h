#include <stdint.h>
#include <stdio.h>

#include <vapoursynth.h>
#include <vsscript.h>
#include <vshelper.h>

#include <memory>
#include <mutex>
#include <tuple>
#include <array>
#include <vector>
#include <queue>
#include <map>
#include <set>
#include <any>
#include <string>
#include <functional>
#include <utility>
#include <type_traits>

#include <QtWidgets>
//#include <QThread>

#define XSTR(X) STR(X)
#define STR(X) #X

namespace enqu {

typedef std::chrono::time_point<std::chrono::high_resolution_clock> time_point_t;

template< typename T>
std::string p2str(const std::any& x);

template< typename T>
int str2p(const char** str, void* x);

template< typename T>
void plane_copy(size_t h, size_t w, uint8_t** ppdst, uint8_t* psrc, size_t src_stride);

template< typename T>
void copy(size_t h, size_t w, size_t np, size_t ssh, size_t ssw, uint8_t* pdst, uint8_t** ppsrc, int* src_stride);

template< typename T>
void plane_copy_s(size_t h, size_t w, uint8_t** ppsrc, uint8_t* pdst, size_t dst_stride);

template< typename T>
void copy_s(size_t h, size_t w, size_t np, size_t ssh, size_t ssw, uint8_t* psrc, uint8_t** ppdst, int* dst_stride);

template< typename T>
void plane_copy_uv(size_t h, size_t w, uint8_t** ppdst, uint8_t* psrc, size_t src_stride);

template< typename T>
void interleave_copy(size_t h, size_t w, size_t np, size_t ssh, size_t ssw, uint8_t* pdst, uint8_t** ppsrc, int* src_stride);

typedef void (*plane_copy_f)(size_t, size_t, uint8_t**, uint8_t*, size_t);
typedef void (*copy_f)(size_t, size_t, size_t, size_t, size_t, uint8_t*, uint8_t**, int*);

extern std::vector<int> g_sof;
extern int g_si, g_nf;
extern QGraphicsPixmapItem* g_pixmap;
extern QLabel* g_stats;
extern QGraphicsView* g_view;
extern char error_msg[1024];
extern const VSAPI* vsapi;
extern VSScript* se;
extern VSCore* core;

struct format
{
	int id;
	int h;
	int w;
	int bit_depth;
	int np;
	int ssx;
	format() = default;
	format(int id, int h, int w);
	format(int h, int w, int bit_depth, int np, int ssx);
	static std::map<std::string, int> name2id;
	static std::map<int, std::string> id2name;
	static int try_get_format_preset(int id)
	{
		const VSFormat* f = vsapi->getFormatPreset(id, core);
		std::string name = f->name;
		if (name.empty())
			return -1;
		id2name[id] = f->name;
		name2id[f->name] = id;
		return 0;
	}
	size_t frame_size() const
	{
		int bytes_per_sample = (bit_depth + 7) >> 3;
		size_t size = (size_t)h * w;
		if (np > 1)
			size += (size >> ssx >> ssx) * (np - 1);
		return size * bytes_per_sample;
	}
};

VSNodeRef* invoke_raws_to_node(uint8_t** ptr);
VSNodeRef* invoke_node_to_src(const format&, VSNodeRef* node);
VSNodeRef* invoke_raws_to_out(const format&, uint8_t** ptr, int, int);
int node_get_frame(int n, VSNodeRef* node, uint8_t** ptr);

int pixmap_update(int si);

struct video_buf_map
{
	virtual uint8_t** src(const format& f) = 0;
	virtual uint8_t* out(int, int, int) = 0;
	virtual uint8_t* out(int, int, uint8_t*, const format& f) = 0;
	virtual ~video_buf_map() = default;
};

typedef std::string(*p2str_t)(const std::any&);
typedef int(*str2p_t)(const char**, void*);

typedef std::chrono::time_point<std::chrono::high_resolution_clock> time_point_t;

#define bool_t int
constexpr bool_t operator"" _b(unsigned long long i) { return bool_t(i); }

struct stats
{
	stats(const std::string& str)
		: str(str)
	{
	}
	std::string str;
	static std::unique_ptr<stats> default_stats(size_t acc_bytes, double elapsed_encode_time)
	{
		char tmp[1024];
		int n = sprintf(tmp, "bitrate = %lf, fps = %lf", .001 * acc_bytes * 8 * 24. / enqu::g_nf, enqu::g_nf / elapsed_encode_time);
		std::string str(tmp, tmp + n);
		return std::make_unique<stats>(str);
	}
};

struct res
{
	format f;
	std::unique_ptr<enqu::stats> stats;
	std::vector<uint8_t*> buf;
	void resize(int id, std::pair<int, int> size, size_t of_count);
	res() = default;
	~res()
	{
		if (!buf.empty())
			free(buf[0]);
	}
	bool empty() const
	{
		return buf.empty();
	}
	uint8_t** data()
	{
		return buf.data();
	}
};

struct key
{
	virtual bool operator<(const key&) const = 0;
	//	virtual ~key() = default;
};

template< typename T>
struct tuple_key_t : key
{
	using tuple_t = T;
	template< size_t I>
	using tuple_element_t = typename std::tuple_element_t<I, tuple_t>;
	tuple_t _;
	static constexpr size_t tuple_size = std::tuple_size_v<tuple_t>, sizeof_tuple = sizeof(tuple_t);
	tuple_key_t() = default;
	tuple_key_t(const tuple_t& _)
		: _(_)
	{
	}
	template< size_t I>
	constexpr const auto& get() const { return std::get<I>(_); }
	template< size_t... index>
	static tuple_t keygen_impl(const size_t* c, const std::vector<std::any>* v, std::index_sequence<index...>)
	{
		return { std::any_cast<std::tuple_element_t<index, tuple_t>>(v[index][c[index]])... };
	}
};

class threadpool;
struct context;
struct encoder
{
	const char* name = 0;
	int err = 0;
	std::string err_detail;
	bool m_abort = 0;
	virtual int encode(context*, threadpool*) = 0;
	virtual ~encoder() = default;
};

struct context
{
	const key* k;
	res& r;
	encoder* e;
	context(const key* k, res& r, encoder* e)
		: k(k), r(r), e(e)
	{
	}
};

class threadpool// : public QThread
{
	//Q_OBJECT
	std::condition_variable v;
	std::mutex mutex;
	std::vector<std::thread> worker;
	std::queue<std::unique_ptr<context>> q;
	bool keep_alive = 0;
	//Q_SIGNALS:
	//void progress_report();
public:
	threadpool()
	{
	}
	~threadpool()
	{
		stop();
	}
	void start(size_t n)
	{
		keep_alive = 1;
		for (size_t i = worker.size(); i < n; i++)
			worker.emplace_back(std::bind(&threadpool::thread, this));
	}
	void stop()
	{
		keep_alive = 0;
		v.notify_all();
		for (std::thread& _ : worker)
			_.join();
		worker.clear();
	}
	void push(std::unique_ptr<context> ctx)
	{
		std::unique_lock lock(mutex);
		q.push(std::move(ctx));
		v.notify_one();
	}
private:
	void thread()
	{
		std::unique_lock lock(mutex, std::defer_lock);
		for (; keep_alive;)
		{
			lock.lock();
			if (q.empty())
			{
				v.wait(lock);
				if (!keep_alive)
				{
					lock.unlock();
					break;
				}
			}
			std::unique_ptr<context> ctx = std::move(q.front());
			q.pop();
			lock.unlock();
			ctx->e->encode(ctx.get(), this);
		}
	}
};

typedef struct par
{
	const char* name;
	p2str_t p2str;
	str2p_t str2p;
	const char* desc;
	//...
} par_t;

struct ctrl
{
	const char* name = 0;
	int err = 0;
	std::string err_detail;
	std::unique_ptr<encoder> e;
	virtual size_t size() const = 0;
	virtual std::unique_ptr<key> keygen(size_t) const = 0;
	template< typename T>
	void update_params_impl(size_t stride, size_t* c, std::vector<std::any>& v, const par_t& p, const char* str)
	{
		std::vector<T> _;
		for (T x; *str && p.str2p(&str, &x);)
			_.emplace_back(x);
		if (_.empty())
			return;
		std::sort(_.begin(), _.end());
		_.erase(std::unique(_.begin(), _.end()), _.end());
		for (size_t i = 0; i < 3; i++)
		{
			if (_.size() > 1)
			{
				T h = std::any_cast<T>(v[c[i * stride]]);
				size_t pos = std::distance(_.begin(), std::find_if(_.begin(), _.end(), [&](T& x) { return !(x < h); }));
				c[i * stride] = std::min(pos, _.size() - 1);
			}
			else
				c[i * stride] = 0;
		}
		v.clear();
		for (size_t i = 0; i < _.size(); i++)
			v.emplace_back(T(_[i]));
	}
	virtual void update(int) = 0;
	virtual void show() {}
	virtual void hide() {}
	virtual ~ctrl() = default;
protected:
	template< typename T, typename ...Args>
	static void default_show(Args...);
	template< typename T, typename ...Args>
	static void default_hide(Args...);
};

class layout
{
	QTabWidget* stack;
public:
	layout(QTabWidget* stack) : stack(stack) {}
	virtual int pixmap_update(int) = 0;
	virtual void input_changed() = 0;
	virtual bool event(QObject*, QEvent*) = 0;
	virtual ~layout() { clear(); }
	void clear()
	{
		while (stack->count())
			delete stack->widget(0);
	}
};

class elabel : public QLabel
{
	Q_OBJECT
public:
	elabel(QWidget* parent = nullptr)
		: QLabel(parent)
	{
	}
Q_SIGNALS:
	void pressed();
protected:
	void mousePressEvent(QMouseEvent*) override
	{
		Q_EMIT pressed();
	}
};

extern format g_f, g_of;
extern std::unique_ptr<video_buf_map> g_buf;
extern std::unique_ptr<layout> g_layout;

}
