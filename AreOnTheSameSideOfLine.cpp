//! Check if p1 and p2 are on the same side of the line determined by points a and b
/*!
* @param p1    Point p1
* @param p2    Point p2
* @param a     First point for determining line
* @param b     Second point for determining line
*/
static int sign(double number) {
    return (number > 0) ? 1 : ((number < 0) ? -1 : 0);
}

static void lineEquationDeterminedByPoints(const cv::Point2f &p, const cv::Point2f &q,
                                           double &a, double &b, double &c) {
    CV_Assert(areEqualPoints(p, q) == false);

    a = q.y - p.y;
    b = p.x - q.x;
    c = ((-p.y) * b) - (p.x * a);
}

static bool areOnTheSameSideOfLine(const cv::Point2f &p1, const cv::Point2f &p2,
                                   const cv::Point2f &a, const cv::Point2f &b) {
    double a1, b1, c1;

    lineEquationDeterminedByPoints(a, b, a1, b1, c1);

    double p1OnLine = (a1 * p1.x) + (b1 * p1.y) + c1;
    double p2OnLine = (a1 * p2.x) + (b1 * p2.y) + c1;

    return (sign(p1OnLine) == sign(p2OnLine));
}
