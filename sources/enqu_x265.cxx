#include "enqu.h"
#include "enqu_x265.h"

#if defined(_WIN32)
#define X265_API_IMPORTS
#endif
#include <x265.h>

namespace enqu {

struct x265_key : tuple_key_t<std::tuple<
	int, float, int, int, int, int,
	int, int, int, bool_t,
	int, float, float, bool_t, int, bool_t, float, int, int, int,
	int, int, int, int, int, int, bool_t, bool_t, bool_t,
	std::array<int, 2>, float, int, float, float, bool_t, std::array<bool_t, 2>, bool_t, int, bool_t, bool_t,
	std::array<int, 2>, std::array<int, 2>, int, int, int, int, bool_t, bool_t, bool_t, bool_t,
	bool_t, std::tuple<int, int, int>, std::tuple<int, int, int>,
	std::tuple<bool_t, int, int>,
	bool_t, bool_t, bool_t, int,
	bool_t, bool_t, std::array<bool_t, 2>, int>>
{
	enum id {
		format_id, bitrate, keyint, min_keyint, gop_lookahead, rc_lookahead,
		bframes, bframe_bias, b_adapt, b_pyramid,
		aq_mode, aq_strength, qp_adaptation_range, aq_motion, qg_size, cutree, qcomp, qpstep, cbqpoffs, crqpoffs,
		max_cu_size, min_cu_size, max_tu_size, tu_intra_depth, tu_inter_depth, limit_tu, weightp, weightb, tskip,
		rd, psy_rd, rdoq_level, psy_rdoq, dynamic_rd, ssim_rd, rd_refine, early_skip, rskip, tskip_fast, splitrd_skip,
		max_merge, ref, limit_refs, me, subme, merange, rect, amp, limit_modes, temporal_mvp,
		hme, hme_search, hme_range,
		deblock,
		sao, sao_non_deblock, limit_sao, selective_sao,
		strong_intra_smoothing, b_intra, fast_intra, rdpenalty,
	};
	x265_key() = default;
	x265_key(const tuple_t& _)
		: tuple_key_t(_)
	{
	}
	static void defaults(std::vector<std::any>* v)
	{
		v[format_id].emplace_back((int)pfYUV420P8);
		v[format_id].emplace_back((int)pfYUV420P10);
		v[bitrate].emplace_back(200.f);
		v[max_cu_size].emplace_back(32);
		v[min_cu_size].emplace_back(8);
		v[max_tu_size].emplace_back(32);
		v[tu_intra_depth].emplace_back(1);
		v[tu_inter_depth].emplace_back(1);
		v[limit_tu].emplace_back(0);
		v[weightp].emplace_back(0_b);
		v[weightb].emplace_back(0_b);
		v[tskip].emplace_back(0_b);
		v[rd].emplace_back(std::array<int, 2>{ 6, 6 });
		v[psy_rd].emplace_back(1.f);
		v[rdoq_level].emplace_back(2);
		v[psy_rdoq].emplace_back(5.f);
		v[dynamic_rd].emplace_back(0.f);
		v[ssim_rd].emplace_back(0_b);
		v[rd_refine].emplace_back(std::array<bool_t, 2>{ 0_b, 0_b });
		v[early_skip].emplace_back(1_b);
		v[rskip].emplace_back(0);
		v[tskip_fast].emplace_back(0_b);
		v[splitrd_skip].emplace_back(0_b);
		v[max_merge].emplace_back(std::array<int, 2>{ 2, 2 });
		v[ref].emplace_back(std::array<int, 2>{ 5, 5 });
		v[limit_refs].emplace_back(1);
		v[me].emplace_back(1);
		v[subme].emplace_back(2);
		v[merange].emplace_back(57);
		v[rect].emplace_back(0_b);
		v[amp].emplace_back(0_b);
		v[limit_modes].emplace_back(0_b);
		v[temporal_mvp].emplace_back(1_b);
		v[hme].emplace_back(0_b);
		v[hme_search].emplace_back(std::make_tuple(1, 1, 3));
		v[hme_range].emplace_back(std::make_tuple(16, 16, 16));
		v[strong_intra_smoothing].emplace_back(0_b);
		v[b_intra].emplace_back(0_b);
		v[fast_intra].emplace_back(std::array<bool_t, 2>{ 1_b, 1_b });
		v[rdpenalty].emplace_back(0);
		v[keyint].emplace_back(250);
		v[min_keyint].emplace_back(0);
		v[gop_lookahead].emplace_back(0);
		v[rc_lookahead].emplace_back(20);
		v[bframes].emplace_back(5);
		v[bframe_bias].emplace_back(0);
		v[b_adapt].emplace_back(2);
		v[b_pyramid].emplace_back(1_b);
		v[aq_mode].emplace_back(3);
		v[aq_strength].emplace_back(1.f);
		v[qp_adaptation_range].emplace_back(1.f);
		v[aq_motion].emplace_back(0_b);
		v[qg_size].emplace_back(16);
		v[cutree].emplace_back(0_b);
		v[qcomp].emplace_back(.6f);
		v[qpstep].emplace_back(4);
		v[cbqpoffs].emplace_back(0);
		v[crqpoffs].emplace_back(0);
		v[deblock].emplace_back(std::make_tuple(0_b, 0, 0));
		v[sao].emplace_back(1_b);
		v[sao_non_deblock].emplace_back(1_b);
		v[limit_sao].emplace_back(0_b);
		v[selective_sao].emplace_back(0);
	}
	bool operator<(const key& x_) const
	{
		const x265_key* x = static_cast<const x265_key*>(&x_);
#define P(N) { if (get<N>() < x->get<N>()) return 1; if (x->get<N>() < get<N>()) return 0; }
		P(format_id);
		P(bitrate);
		P(keyint);
		P(min_keyint);
		P(gop_lookahead);
		P(rc_lookahead);
		P(bframes);
		P(bframe_bias);
		P(b_adapt);
		P(b_pyramid);
		P(aq_mode);
		P(aq_strength);
		P(qp_adaptation_range);
		P(aq_motion);
		P(qg_size);
		P(cutree);
		P(qcomp);
		P(qpstep);
		P(cbqpoffs);
		P(crqpoffs);
		P(max_cu_size);
		P(min_cu_size);
		P(max_tu_size);
		P(tu_intra_depth);
		P(tu_inter_depth);
		P(limit_tu);
		P(weightp);
		P(weightb);
		P(tskip);
		P(rd);
		P(rdoq_level);
		if (get<rdoq_level>())
			P(psy_rdoq);
		P(dynamic_rd);
		P(ssim_rd);
		if (!get<ssim_rd>())
			P(psy_rd);
		P(rd_refine);
		P(early_skip);
		P(rskip);
		P(tskip_fast);
		P(splitrd_skip);
		P(max_merge);
		P(ref);
		P(limit_refs);
		P(subme);
		P(rect);
		P(amp);
		P(limit_modes);
		P(temporal_mvp);
		P(hme);
		if (get<hme>())
		{
			P(hme_search);
			P(hme_range);
		}
		else
		{
			P(me);
			P(merange);
		}
		P(deblock);
		P(sao);
		P(sao_non_deblock);
		P(limit_sao);
		P(selective_sao);
		P(strong_intra_smoothing);
		P(b_intra);
		P(fast_intra);
		P(rdpenalty);
#undef P
		return 0;
	}
	std::unique_ptr<context> ctx(res& res, encoder* e) const
	{
		return std::make_unique<context>(static_cast<const enqu::key*>(this), res, e);
	}
};

struct x265_encoder : encoder
{
	x265_encoder();
	~x265_encoder();
	int encode(context*, threadpool*);
	static void param_apply_key(::x265_param*, const x265_key*, int);
};

x265_encoder::x265_encoder()
{
}

x265_encoder::~x265_encoder()
{
}

void x265_encoder::param_apply_key(::x265_param* p, const x265_key* k, int pass)
{
	p->rc.rateControlMode = X265_RC_ABR;
	p->rc.bitrate = k->get<x265_key::bitrate>();

	p->maxCUSize = k->get<x265_key::max_cu_size>();
	p->minCUSize = k->get<x265_key::min_cu_size>();
	p->maxTUSize = k->get<x265_key::max_tu_size>();
	p->tuQTMaxIntraDepth = k->get<x265_key::tu_intra_depth>();
	p->tuQTMaxInterDepth = k->get<x265_key::tu_inter_depth>();
	p->limitTU = k->get<x265_key::limit_tu>();

	p->bEnableWeightedPred = k->get<x265_key::weightp>();
	p->bEnableWeightedBiPred = k->get<x265_key::weightb>();
	p->bEnableTransformSkip = k->get<x265_key::tskip>();

	p->rdLevel = k->get<x265_key::rd>()[0];
	p->psyRd = k->get<x265_key::psy_rd>();
	p->rdoqLevel = k->get<x265_key::rdoq_level>();
	p->psyRdoq = k->get<x265_key::psy_rdoq>();
	p->dynamicRd = k->get<x265_key::dynamic_rd>();
	if (k->get<x265_key::ssim_rd>())
		p->bSsimRd = 1, p->psyRd = 0.f;
	p->bEnableRdRefine = k->get<x265_key::rd_refine>()[0];
	p->bEnableEarlySkip = k->get<x265_key::early_skip>();
	p->recursionSkipMode = k->get<x265_key::rskip>();
	p->bEnableTSkipFast = k->get<x265_key::tskip_fast>();
	p->bEnableSplitRdSkip = k->get<x265_key::splitrd_skip>();

	p->maxNumMergeCand = k->get<x265_key::max_merge>()[0];
	p->maxNumReferences = k->get<x265_key::ref>()[0];
	p->limitReferences = k->get<x265_key::limit_refs>();
	p->searchMethod = k->get<x265_key::me>();
	p->subpelRefine = k->get<x265_key::subme>();
	p->searchRange = k->get<x265_key::merange>();
	p->bEnableRectInter = k->get<x265_key::rect>();
	if (k->get<x265_key::amp>())
		p->bEnableAMP = p->bEnableRectInter = 1;
	p->limitModes = k->get<x265_key::limit_modes>();
	p->bEnableTemporalMvp = k->get<x265_key::temporal_mvp>();
	p->bEnableHME = k->get<x265_key::hme>();
	std::tie(p->hmeSearchMethod[0], p->hmeSearchMethod[1], p->hmeSearchMethod[2]) = k->get<x265_key::hme_search>();
	std::tie(p->hmeRange[0], p->hmeRange[1], p->hmeRange[2]) = k->get<x265_key::hme_range>();

	p->bEnableStrongIntraSmoothing = k->get<x265_key::strong_intra_smoothing>();
	p->bIntraInBFrames = k->get<x265_key::b_intra>();
	p->bEnableFastIntra = k->get<x265_key::fast_intra>()[0];
	p->rdPenalty = k->get<x265_key::rdpenalty>();

	p->keyframeMax = k->get<x265_key::keyint>();
	p->keyframeMin = k->get<x265_key::min_keyint>();
	p->gopLookahead = k->get<x265_key::gop_lookahead>();
	p->lookaheadDepth = k->get<x265_key::rc_lookahead>();

	p->bframes = k->get<x265_key::bframes>();
	p->bFrameBias = k->get<x265_key::bframe_bias>();
	p->bFrameAdaptive = k->get<x265_key::b_adapt>();
	p->bBPyramid = k->get<x265_key::b_pyramid>();

	p->rc.aqMode = k->get<x265_key::aq_mode>();
	p->rc.aqStrength = k->get<x265_key::aq_strength>();
	p->rc.qpAdaptationRange = k->get<x265_key::qp_adaptation_range>();
	p->bAQMotion = k->get<x265_key::aq_motion>();
	p->rc.qgSize = k->get<x265_key::qg_size>();
	p->rc.cuTree = k->get<x265_key::cutree>();
	p->rc.qCompress = k->get<x265_key::qcomp>();
	p->rc.qpStep = k->get<x265_key::qpstep>();
	p->cbQpOffset = k->get<x265_key::cbqpoffs>();
	p->crQpOffset = k->get<x265_key::crqpoffs>();

	std::tie(p->bEnableLoopFilter, p->deblockingFilterTCOffset, p->deblockingFilterBetaOffset) = k->get<x265_key::deblock>();
	p->bEnableSAO = k->get<x265_key::sao>();
	p->bSaoNonDeblocked = k->get<x265_key::sao_non_deblock>();
	p->bLimitSAO = k->get<x265_key::limit_sao>();
	p->selectiveSAO = k->get<x265_key::selective_sao>();

	if (pass)
	{
		p->maxNumMergeCand = k->get<x265_key::max_merge>()[1];
		p->maxNumReferences = k->get<x265_key::ref>()[1];
		p->bEnableFastIntra = k->get<x265_key::fast_intra>()[1];
		p->rdLevel = k->get<x265_key::rd>()[1];
		p->bEnableRdRefine = k->get<x265_key::rd_refine>()[1];
	}
}

static int f2f(const format& f, int& csp)
{
	switch (f.np) {
	case 1: csp = X265_CSP_I400; break;
	case 3:
		switch (f.ssx) {
		case 0: csp = X265_CSP_I444; break;
		case 1: csp = X265_CSP_I420; break;
		default: return -1;
		} break;
	default:
		return -1;
	}
	return 0;
}

int x265_encoder::encode(context* ctx, threadpool*)
{
	const x265_key* k = static_cast<const x265_key*>(ctx->k);
	int id = k->get<x265_key::format_id>();
	ctx->r.resize(id, std::make_pair(g_f.w, g_f.h), g_sof.size());
	x265_picture pic_in, pic_out;
	x265_nal* p_nal;
	uint32_t i_nal;
	size_t acc_bytes = 0;
	time_point_t t0 = std::chrono::high_resolution_clock::now();
	for (int pass = 0; pass < 2; pass++)
	{
		const format& f = ctx->r.f;
		int csp;
		if (f2f(f, csp))
			return -1;
		const ::x265_api* api = x265_api_query(f.bit_depth, 179, 0);
		if (!api)
			return -1;
		int css = f.bit_depth == 8 ? 0 : 1;
		copy_f copy = !css ? enqu::copy<uint8_t> : enqu::copy<uint16_t>;
		::x265_param p;
		//api->param_default_preset(&p, "ultrafast", 0);
		api->param_default(&p);
		p.internalCsp = csp;
		p.sourceBitDepth = f.bit_depth;
		p.sourceWidth = f.w;
		p.sourceHeight = f.h;
		p.totalFrames = g_nf;
		p.frameNumThreads = 1;
		p.lookaheadSlices = 0;
		p.numaPools = "none";
		p.bEnableWavefront = 0;
		p.fpsNum = 24000;
		p.fpsDenom = 1001;
		p.bEnablePsnr = 0;
		p.bAllowNonConformance = 1;
		p.bCopyPicToFrame = 1;
		p.logLevel = X265_LOG_NONE;
		if (pass)
			p.rc.bStatRead = 2;
		else
			p.rc.bStatWrite = 1;
		api->picture_init(&p, &pic_in);
		pic_in.colorSpace = csp;
		pic_in.bitDepth = f.bit_depth;
		pic_in.stride[0] = f.w << css;
		pic_in.stride[2] = pic_in.stride[1] = pic_in.stride[0] >> f.ssx;
		int u_off = pic_in.stride[0] * f.h;
		int v_off = u_off + (pic_in.stride[1] * f.h >> f.ssx);
		param_apply_key(&p, k, pass);
		::x265_encoder* e = api->encoder_open(&p);
		if (!e)
			return -1;
		uint8_t** pp = g_buf->src(f);
		::x265_picture* ppic_in = &pic_in, * ppic_out = &pic_out;
		for (int i = 0, j = 0; j < g_nf;)
		{
			if (ppic_in)
			{
				if (i < g_nf)
				{
					uint8_t* p = *pp++;
					ppic_in->planes[0] = p;
					ppic_in->planes[1] = p + u_off;
					ppic_in->planes[2] = p + v_off;
					i++;
				}
				else
					ppic_in = 0;
			}
			int n = api->encoder_encode(e, &p_nal, &i_nal, ppic_in, ppic_out);
			if (n < 0 || m_abort)
			{
				api->encoder_close(e);
				return -1;
			}
			j += n;
			if (!pass)
				continue;
			for (int i = 0; i < i_nal; i++)
				acc_bytes += p_nal[i].sizeBytes;
			if (ppic_out && n)
			{
				size_t n = std::distance(g_sof.begin(), std::find(g_sof.begin(), g_sof.end(), ppic_out->poc));
				if (n < g_sof.size())
					copy(f.h, f.w, f.np, f.ssx, f.ssx, ctx->r.data()[n], (uint8_t**)ppic_out->planes, ppic_out->stride);
			}
		}
		api->encoder_close(e);
		api->cleanup();
	}
	double elapsed_encode_time = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t0).count();
	ctx->r.stats = stats::default_stats(acc_bytes, elapsed_encode_time);
	return 0;
}

struct x265_ctrl : ctrl
{
	using key_t = x265_key;
	std::vector<std::any> v[key_t::tuple_size];
	size_t ctrl[3][key_t::tuple_size] = {};
	size_t size() const { return key_t::tuple_size; }
	std::unique_ptr<key> keygen(size_t i) const
	{
		return std::make_unique<key_t>(key_t::keygen_impl(ctrl[i], v, std::make_index_sequence<key_t::tuple_size>{}));
	}
	template< size_t I = 0>
	void update_params(size_t i, const std::string& str)
	{
		if constexpr (I < key_t::tuple_size)
		{
			if (i != I)
				update_params<I + 1>(i, str);
			else
				update_params_impl<key_t::tuple_element_t<I>>(key_t::tuple_size, (size_t*)ctrl + i, v[i], p[i], str.c_str());
		}
	}
	static const par_t p[/*key_t::tuple_size*/];
	QGridLayout* g;
	QDialog* d;
	QPlainTextEdit* t;
	QSlider* s[key_t::tuple_size];
	QLabel* s0[key_t::tuple_size], * s1[key_t::tuple_size];
	int i = 0, j = 0;
	x265_ctrl(QGridLayout* g, QWidget* parent)
		: g(g)
	{
		name = "x265";
		e = std::make_unique<enqu::x265_encoder>();
		if (e->err)
		{
			err = -1;
			return;
		}
		key_t::defaults(v);
		d = new QDialog(parent);
		d->setWindowModality(Qt::ApplicationModal);
		d->resize(256, 256);
		t = new QPlainTextEdit(d);
		t->setGeometry(QRect(0, 0, 256, 256));
		QObject::connect(d, &QDialog::finished, [this](int)
		{
			std::string text = t->toPlainText().toStdString();
			update_params(i, text);
			s[i]->setRange(0, v[i].size() - 1);
			update_slider(i);
			g_layout->pixmap_update(g_si);
		});
		for (int i = 0; i < key_t::tuple_size; i++)
		{
			auto q2 = new QSlider(Qt::Orientation::Horizontal);
			q2->setTickPosition(QSlider::TickPosition::TicksBothSides);
			q2->setRange(0, v[i].size() - 1);
			q2->setFixedWidth(100);
			auto q1 = new QLabel(p[i].name);
			if (p[i].desc)
				q1->setToolTip(p[i].desc);
			q1->setFixedWidth(100);
			auto q3 = new elabel();
			q3->setBackgroundRole(QPalette::BrightText);
			q3->setAutoFillBackground(true);
			QObject::connect(q3, &elabel::pressed, [this, i]()
			{
				std::string text;
				for (int j = 0; j < v[i].size(); j++)
					text += p[i].p2str(v[i][j]) + '\n';
				t->setPlainText(QString::fromStdString(text));
				t->moveCursor(QTextCursor::MoveOperation::End);
				t->setWindowTitle(QString::fromStdString(p[i].name));
				this->i = i; // save
				d->show();
			});
			QObject::connect(q2, &QAbstractSlider::sliderMoved, [this, i](int n)
			{
				update_string(i, n);
				ctrl[j][i] = n;
				g_layout->pixmap_update(g_si);
			});
			s0[i] = q1;
			s[i] = q2;
			s1[i] = q3;
			g->addWidget(q1, i, 0);
			g->addWidget(q2, i, 1);
			g->addWidget(q3, i, 2);
		}
	}
	void update_string(int i, int j)
	{
		if (s1[i])
			s1[i]->setText(QString::fromStdString(p[i].p2str(v[i][j])));
	}
	void update_slider(int i)
	{
		if (s[i])
		{
			int n = ctrl[j][i];
			s[i]->setSliderPosition(n);
			bool enabled = v[i].size() > 1;
			if (s[i]->isEnabled() == !enabled)
				s[i]->setDisabled(!enabled);
			update_string(i, n);
		}
	}
	void update(int j_)
	{
		j = j_;
		for (int i = 0; i < key_t::tuple_size; i++)
			update_slider(i);
	}
};

std::unique_ptr<ctrl> make_x265_ctrl(QGridLayout* g, QWidget* parent)
{
	return std::make_unique<x265_ctrl>(g, parent);
}

#define X(T) enqu::p2str<x265_key::tuple_element_t<x265_key::T>>, enqu::str2p<x265_key::tuple_element_t<x265_key::T>>
const par_t x265_ctrl::p[] = {
{ "format", enqu::p2str<format>, enqu::str2p<format> },
{ "bitrate", X(bitrate) },
{ "keyint", X(keyint) },
{ "min_keyint", X(min_keyint) },
{ "gop_lookahead", X(gop_lookahead) },
{ "rc_lookahead", X(rc_lookahead) },
{ "bframes", X(bframes) },
{ "bframe_bias", X(bframe_bias) },
{ "b_adapt", X(b_adapt) },
{ "b_pyramid", X(b_pyramid) },
{ "aq_mode", X(aq_mode) },
{ "aq_strength", X(aq_strength) },
{ "qp_adaptation_range", X(qp_adaptation_range) },
{ "aq_motion", X(aq_motion) },
{ "qg_size", X(qg_size) },
{ "cutree", X(cutree) },
{ "qcomp", X(qcomp) },
{ "qpstep", X(qpstep) },
{ "cbqpoffs", X(cbqpoffs) },
{ "crqpoffs", X(crqpoffs) },
{ "max_cu_size", X(max_cu_size) },
{ "min_cu_size", X(min_cu_size) },
{ "max_tu_size", X(max_tu_size) },
{ "tu_intra_depth", X(tu_intra_depth) },
{ "tu_inter_depth", X(tu_inter_depth) },
{ "limit_tu", X(limit_tu) },
{ "weightp", X(weightp) },
{ "weightb", X(weightb) },
{ "tskip", X(tskip) },
{ "rd", X(rd) },
{ "psy_rd", X(psy_rd) },
{ "rdoq_level", X(rdoq_level) },
{ "psy_rdoq", X(psy_rdoq) },
{ "dynamic_rd", X(dynamic_rd) },
{ "ssim_rd", X(ssim_rd) },
{ "rd_refine", X(rd_refine) },
{ "early_skip", X(early_skip) },
{ "rskip", X(rskip) },
{ "tskip_fast", X(tskip_fast) },
{ "splitrd_skip", X(splitrd_skip) },
{ "max_merge", X(max_merge) },
{ "ref", X(ref) },
{ "limit_refs", X(limit_refs) },
{ "me", X(me), "ME search method (DIA, HEX, UMH, STAR, SEA, FULL). The search patterns (methods) are sorted in increasing complexity, with diamond being the simplest and fastest and full being the slowest.  DIA, HEX, UMH and SEA were adapted from x264 directly. STAR is an adaption of the HEVC reference encoder's three step search, while full is a naive exhaustive search. The default is the star search, it has a good balance of performance and compression efficiency" },
{ "subme", X(subme) },
{ "merange", X(merange), "The maximum distance from the motion prediction that the full pel motion search is allowed to progress before terminating. This value can have an effect on frame parallelism, as referenced frames must be at least this many rows of reconstructed pixels ahead of the referencee at all times. (When considering reference lag, the motion prediction must be ignored because it cannot be known ahead of time).  Default is 60, which is the default max CU size (64) minus the luma HPEL half-filter length (4). If a smaller CU size is used, the search range should be similarly reduced" },
{ "rect", X(rect) },
{ "amp", X(amp) },
{ "limit_modes", X(limit_modes) },
{ "temporal_mvp", X(temporal_mvp) },
{ "hme", X(hme) },
{ "hme_search", X(hme_search) },
{ "hme_range", X(hme_range) },
{ "deblock", X(deblock) },
{ "sao", X(sao) },
{ "sao_non_deblock", X(sao_non_deblock) },
{ "limit_sao", X(limit_sao) },
{ "selective_sao", X(selective_sao) },
{ "strong_intra_smoothing", X(strong_intra_smoothing) },
{ "b_intra", X(b_intra) },
{ "fast_intra", X(fast_intra) },
{ "rdpenalty", X(rdpenalty) },
};
#undef X
static_assert(x265_key::tuple_size == sizeof(x265_ctrl::p) / sizeof(par_t));

class x265_layout : public layout
{
	QScrollArea* scroll;
	QGridLayout* grid;
	std::unique_ptr<enqu::ctrl> ctrl;
	std::map<x265_key, res> q;
	std::unique_ptr<threadpool> pool;
	int cj = -1;
public:
	x265_layout(QTabWidget*);
	~x265_layout()
	{
	}
	int pixmap_update(int);
	void input_changed();
	bool event(QObject*, QEvent*);
protected:
	void process(int);
	void cj_changed(int);
};

std::unique_ptr<layout> make_x265_layout(QTabWidget* tab)
{
	return std::make_unique<x265_layout>(tab);
}

void x265_layout::cj_changed(int n)
{
	int _cj = std::abs(cj);
	if (n)
	{
		if (cj < 0)
			scroll->setDisabled(0);
		if (_cj != n)
			ctrl->update(n - 1);
		cj = n;
	}
	else
	{
		if (cj > 0)
			scroll->setDisabled(1);
		cj = -_cj;
	}
	pixmap_update(g_si);
}

x265_layout::x265_layout(QTabWidget* stack)
	: layout(stack)
{
	pool = std::make_unique<threadpool>();
	pool->start(1);
	scroll = new QScrollArea;
	QWidget* w = new QWidget;
	grid = new QGridLayout(w);
	grid->setSizeConstraint(QLayout::SetMinAndMaxSize);
	ctrl = make_x265_ctrl(grid, scroll);
	if (ctrl->err)
		ctrl.reset();
	else
		ctrl->update(0);
	scroll->setDisabled(true);
	scroll->setWidget(w);
	stack->addTab(scroll, "");
}

bool x265_layout::event(QObject* obj, QEvent* event)
{
	int type = event->type();
	if (type == QEvent::KeyPress)
	{
		QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
		int key = key_event->key(), modifiers = key_event->modifiers();
		switch (key)
		{
		case Qt::Key_1:
			cj_changed(0);
			break;
		case Qt::Key_2:
			cj_changed(1);
			break;
		case Qt::Key_3:
			cj_changed(2);
			break;
		case Qt::Key_4:
			cj_changed(3);
			break;
		case Qt::Key_F5:
			if (cj > 0 && g_buf) process(0);
			break;
		default:
			return 0;
		}
		return 1;
	}
	return 0;
}

void x265_layout::input_changed()
{
	q.clear();
}

int x265_layout::pixmap_update(int si)
{
	if (cj < 0)
		return enqu::pixmap_update(si);
	auto pk = ctrl->keygen(cj - 1);
	auto& k = *static_cast<x265_key*>(pk.get());
	if (!q.count(k) || q.at(k).empty())
	{
		QPixmap px;
		g_pixmap->setPixmap(px);
		g_stats->setText(QString());
	}
	else
	{
		auto v = &q.at(k);
		int w = g_of.w, h = g_of.h;
		g_pixmap->setPixmap(QPixmap::fromImage(QImage((const uchar*)g_buf->out(h, w, v->buf[si], v->f), w, h, QImage::Format_RGB32)));
		if (v->stats)
			g_stats->setText(QString::fromStdString(v->stats->str));
	}
	return 0;
}

void x265_layout::process(int)
{
	auto pk = ctrl->keygen(cj - 1);
	auto& k = *static_cast<x265_key*>(pk.get());
	auto t = q.try_emplace(k);
	if (t.second)
		pool->push(t.first->first.ctx(t.first->second, ctrl->e.get()));
}

}
