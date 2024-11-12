/*
pt2-pt1向量有以下四种形式:其中↙↖，dy取反，pt1和pt2交换
↘↙
↗↖

*/

#define XY_ONE 65536
#define XY_SHIFT 16
void m_ellipse2Poly(Point2d center, Size2d axes, int angle,
	int arc_start, int arc_end,
	int delta, std::vector<Point2d>& pts)
{
	CV_Assert(0 < delta && delta <= 180);

	float alpha, beta;
	int i;

	while (angle < 0)
		angle += 360;
	while (angle > 360)
		angle -= 360;

	if (arc_start > arc_end)
	{
		i = arc_start;
		arc_start = arc_end;
		arc_end = i;
	}
	while (arc_start < 0)
	{
		arc_start += 360;
		arc_end += 360;
	}
	while (arc_end > 360)
	{
		arc_end -= 360;
		arc_start -= 360;
	}
	if (arc_end - arc_start > 360)
	{
		arc_start = 0;
		arc_end = 360;
	}
	//sincos(angle, alpha, beta);
	alpha = std::cos(angle * CV_PI / 180);
	beta = std::sin(angle * CV_PI / 180);
	pts.resize(0);

	for (i = arc_start; i < arc_end + delta; i += delta)
	{
		double x, y;
		angle = i;
		if (angle > arc_end)
			angle = arc_end;
		if (angle < 0)
			angle += 360;

		x = axes.width * std::cos(angle*CV_PI/180);
		y = axes.height * std::sin(angle * CV_PI / 180);
		Point2d pt;
		pt.x = center.x + x * alpha - y * beta;
		pt.y = center.y + x * beta + y * alpha;
		pts.push_back(pt);
	}

	// If there are no points, it's a zero-size polygon
	if (pts.size() == 1) {
		pts.assign(2, center);
	}
}

//直线画法，x++,y+delta*1或者y++,x+delata*1(delta==dy/dx)
static void
Line2(Mat& img, Point2l pt1, Point2l pt2, const void* color)
{
	int64 dx, dy;
	int ecount;
	int64 ax, ay;
	int64 i, j;
	int x, y;
	int64 x_step, y_step;
	int cb = ((uchar*)color)[0];
	int cg = ((uchar*)color)[1];
	int cr = ((uchar*)color)[2];
	int pix_size = (int)img.elemSize();
	uchar* ptr = img.ptr(), * tptr;
	size_t step = img.step;
	Size size = img.size();

	//CV_Assert( img && (nch == 1 || nch == 3) && img.depth() == CV_8U );

	Size2l sizeScaled(((int64)size.width) << XY_SHIFT, ((int64)size.height) << XY_SHIFT);
	if (!clipLine(sizeScaled, pt1, pt2))
		return;

	dx = pt2.x - pt1.x;
	dy = pt2.y - pt1.y;

	//求dx,dy绝对值的位运算方法
	//^(-1):负数取反-1，^(0)不变
	j = dx < 0 ? -1 : 0;
	ax = (dx ^ j) - j;
	i = dy < 0 ? -1 : 0;
	ay = (dy ^ i) - i;

  //ecount计算画直线迭代次数
  //保证pt1<pt2从小画线到大
  //(|1)防止ax为0
	if (ax > ay)
	{
		dy = (dy ^ j) - j;
		pt1.x ^= pt2.x & j;
		pt2.x ^= pt1.x & j;
		pt1.x ^= pt2.x & j;
		pt1.y ^= pt2.y & j;
		pt2.y ^= pt1.y & j;
		pt1.y ^= pt2.y & j;

		x_step = XY_ONE;
		y_step = dy * (1 << XY_SHIFT) / (ax | 1);//
		ecount = (int)((pt2.x - pt1.x) >> XY_SHIFT);
	}
	else
	{
		dx = (dx ^ i) - i;
		pt1.x ^= pt2.x & i;
		pt2.x ^= pt1.x & i;
		pt1.x ^= pt2.x & i;
		pt1.y ^= pt2.y & i;
		pt2.y ^= pt1.y & i;
		pt1.y ^= pt2.y & i;

		x_step = dx * (1 << XY_SHIFT) / (ay | 1);//Δx增长画栅格点
		y_step = XY_ONE;//dy长，y++增长
		ecount = (int)((pt2.y - pt1.y) >> XY_SHIFT);
	}

  //四舍五入
	pt1.x += (XY_ONE >> 1);
	pt1.y += (XY_ONE >> 1);

	cout << "pt1xy: " << pt1 << endl;

	#define  ICV_PUT_POINT(_x,_y) \
        x = (_x); y = (_y);           \
        if( 0 <= x && x < size.width && \
            0 <= y && y < size.height ) \
        {                           \
            tptr = ptr + y*step + x;\
            tptr[0] = (uchar)cb;    \
			cout<<"xyinput: "<<x<<" "<<y<<endl;\
        }

	ICV_PUT_POINT((int)((pt2.x + (XY_ONE >> 1)) >> XY_SHIFT),
		(int)((pt2.y + (XY_ONE >> 1)) >> XY_SHIFT));

	if (ax > ay)
	{
		pt1.x >>= XY_SHIFT;

		while (ecount >= 0)
		{
			ICV_PUT_POINT((int)(pt1.x), (int)(pt1.y >> XY_SHIFT));
			pt1.x++;
			pt1.y += y_step;//dy/|dx|
			ecount--;
		}
	}
	else
	{
		pt1.y >>= XY_SHIFT;

		while (ecount >= 0)
		{
			ICV_PUT_POINT((int)(pt1.x >> XY_SHIFT), (int)(pt1.y));
			pt1.x += x_step;
			pt1.y++;
			ecount--;
			cout << "pt1x: " << pt1.x << endl;
		}
	}

#undef ICV_PUT_POINT
}

//opencv直线画法
static void
Line(Mat& img, Point pt1, Point pt2,
	const void* _color, int connectivity = 8)
{
	if (connectivity == 0)
		connectivity = 8;
	else if (connectivity == 1)
		connectivity = 4;

	LineIterator iterator(img, pt1, pt2, connectivity, true);
	int i, count = iterator.count;
	int pix_size = (int)img.elemSize();
	const uchar* color = (const uchar*)_color;

	if (pix_size == 3)
	{
		for (i = 0; i < count; i++, ++iterator)
		{
			uchar* ptr = *iterator;
			ptr[0] = color[0];
			ptr[1] = color[1];
			ptr[2] = color[2];
		}
	}
	else
	{
		for (i = 0; i < count; i++, ++iterator)
		{
			uchar* ptr = *iterator;
			if (pix_size == 1)
				ptr[0] = color[0];
			else
				memcpy(*iterator, color, pix_size);
		}
	}
}

static void
ThickLine(Mat& img, Point2l p0, Point2l p1, const void* color,
	int thickness, int line_type, int flags, int shift)
{
	static const double INV_XY_ONE = 1. / XY_ONE;

	p0.x <<= XY_SHIFT - shift;
	p0.y <<= XY_SHIFT - shift;
	p1.x <<= XY_SHIFT - shift;
	p1.y <<= XY_SHIFT - shift;

	if (line_type == 1 || line_type == 4 || shift == 0)
	{
		p0.x = (p0.x + (XY_ONE >> 1)) >> XY_SHIFT;
		p0.y = (p0.y + (XY_ONE >> 1)) >> XY_SHIFT;
		p1.x = (p1.x + (XY_ONE >> 1)) >> XY_SHIFT;
		p1.y = (p1.y + (XY_ONE >> 1)) >> XY_SHIFT;
		Line(img, p0, p1, color, line_type);
	}
	else
		Line2(img, p0, p1, color);
}


static void
PolyLine(Mat& img, const Point2l* v, int count, bool is_closed,
	const void* color, int thickness,
	int line_type, int shift)
{
	if (!v || count <= 0)
		return;

	int i = is_closed ? count - 1 : 0;
	int flags = 2 + !is_closed;
	Point2l p0;
	CV_Assert(0 <= shift && shift <= XY_SHIFT && thickness >= 0);

  //如果轮廓要闭合，连接最后一个v[n-1]与首个!is_closed(1)==v[0]
  //不闭合的时候，连接第一个v[0]与第二个!is_closed(0)==v[1]
	p0 = v[i];
	for (i = !is_closed; i < count; i++)
	{
		Point2l p = v[i];
    //连接线段
		ThickLine(img, p0, p, color, thickness, line_type, flags, shift);
		p0 = p;
		flags = 2;
	}
}

static void
EllipseEx(Mat& img, Point2l center, Size2l axes,
	int angle, int arc_start, int arc_end,
	const void* color, int thickness, int line_type)
{
	axes.width = std::abs(axes.width), axes.height = std::abs(axes.height);
	int delta = (int)((std::max(axes.width, axes.height) + (XY_ONE >> 1)) >> XY_SHIFT);
  //小于3栅格，10栅格，15栅格，以及更大
	delta = delta < 3 ? 90 : delta < 10 ? 30 : delta < 15 ? 18 : 5;

  //按照角度每取delta算一个弧段，画出连续的线段近似拟合圆弧  
	std::vector<Point2d> _v;
	m_ellipse2Poly(Point2d((double)center.x, (double)center.y), Size2d((double)axes.width, (double)axes.height), angle, arc_start, arc_end, delta, _v);

	std::vector<Point2l> v;
	Point2l prevPt(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
	v.resize(0);
	for (unsigned int i = 0; i < _v.size(); ++i)
	{
		Point2l pt;
		pt.x = (int64)cvRound(_v[i].x / XY_ONE) << XY_SHIFT;
		pt.y = (int64)cvRound(_v[i].y / XY_ONE) << XY_SHIFT;
		pt.x += cvRound(_v[i].x - pt.x);
		pt.y += cvRound(_v[i].y - pt.y);
		if (pt != prevPt) {
			v.push_back(pt);
			prevPt = pt;
		}
	}

	for (auto contour : v) cout << contour << endl;

	// If there are no points, it's a zero-size polygon
	if (v.size() == 1) {
		v.assign(2, center);
	}

	if (thickness >= 0)
		PolyLine(img, &v[0], (int)v.size(), false, color, thickness, line_type, XY_SHIFT);
}
