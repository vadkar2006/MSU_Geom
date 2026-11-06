#pragma once

#include <QPointF>
#include <QString>
#include <type_traits>
#include <utility>
#include <QVector>


namespace geom_detail {

template <typename U, typename = void>
struct HasToDouble : std::false_type {};
template <typename U>
struct HasToDouble<U, std::void_t<decltype(std::declval<const U&>().toDouble())>> : std::true_type {};

template <typename U>
inline double toDouble(const U& v) {
    if constexpr (std::is_same_v<U, double>)      return v;
    else if constexpr (std::is_same_v<U, float>)  return static_cast<double>(v);
    else if constexpr (HasToDouble<U>::value)     return v.toDouble();
    else                                          return static_cast<double>(v);
}

}

template <typename T = double>
struct Point {
    T x{};
    T y{};

    Point() = default;
    Point(const T& x_, const T& y_) : x(x_), y(y_) {}

    QPointF toQPointF() const { return QPointF(geom_detail::toDouble(x), geom_detail::toDouble(y)); }

    Point<double> toDoublePoint() const { return Point<double>(geom_detail::toDouble(x), geom_detail::toDouble(y)); }

    QString toString(int decimals = 2) const {
        return QString("(%1, %2)").arg(geom_detail::toDouble(x), 0, 'f', decimals)
                                  .arg(geom_detail::toDouble(y), 0, 'f', decimals);
    }

    Point operator+(const Point& o) const { return Point(x + o.x, y + o.y); }
    Point operator-(const Point& o) const { return Point(x - o.x, y - o.y); }
    bool operator==(const Point& o) const { return x == o.x && y == o.y; }
    bool operator!=(const Point& o) const { return !(*this == o); }
};

template <typename T>
inline QVector<Point<double>> toDoubleVector(const QVector<Point<T>>& pts) {
    QVector<Point<double>> res;
    res.reserve(pts.size());
    for (const auto& p : pts) {
        res.push_back(p.toDoublePoint());
    }
    return res;
}

using PointD = Point<double>;
