//点到直线距离
static double distanceFromPointToLine(const cv::Point2f &a, const cv::Point2f &linePointB,
                                      const cv::Point2f &linePointC) {
    double term1 = linePointC.x - linePointB.x;
    double term2 = linePointB.y - a.y;
    double term3 = linePointB.x - a.x;
    double term4 = linePointC.y - linePointB.y;

    double nominator = std::abs((term1 * term2) - (term3 * term4));
    double denominator = std::sqrt((term1 * term1) + (term4 * term4));

    return (denominator != 0) ? (nominator / denominator)
                              : 0;
}

std::vector<cv::Point> BresenhemnLine(cv::Point p1, cv::Point p2)
{
    std::vector<cv::Point> output_line;
    std::vector<cv::Point> rect_pts{p1, p2};
    cv::Rect rect = cv::boundingRect(rect_pts);
    cv::Point offset{rect.x, rect.y};
    cv::Mat src(rect.height, rect.width, 0, cv::Scalar(0));

    cv::line(src, p1 - offset, p2 - offset, 1);
    cv::findNonZero(src, output_line);

    for (auto &pt : output_line)
        pt = pt + offset;

    return output_line;
}
