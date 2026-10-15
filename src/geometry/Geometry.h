#pragma once
#include "Point.h"


namespace predicate
{
    inline int orientation(const Point& a, const Point& b, const Point& c, double epsilon = 1e-9)
    {
        double cross = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);

        if (cross >  epsilon) return  1;
        if (cross < -epsilon) return -1;
        return 0;
    }

    inline const char* orientationNameRu(int o)
    {
        switch (o) {
        case  1: return "По часовой стрелке (правый поворот)";
        case -1: return "Против часовой стрелки (левый поворот)";
        case  0: return "Коллинеарны";
        }
        return "Неизвестно";
    }
}


namespace segment_intersection
{
    //При условии предикат = 0
    inline bool on_segment(const Point& a, const Point& b, const Point& c, double epsilon = 1e-9)
    {
        bool flag1x =  std::min(a.x, b.x) <= c.x;
        bool flag2x =  c.x <= std::max(a.x, b.x);

        bool flag1y =  std::min(a.y, b.y) <= c.y;
        bool flag2y =  c.y <= std::max(a.y, b.y);

        return flag1x && flag2x && flag1y && flag2y;
    }

    inline int orientation_segment(const Point& a, const Point& b, const Point& c, const Point& d, double epsilon = 1e-9)
    {
        int predecate_abc = predicate::orientation(a, b, c, epsilon);
        int predecate_abd = predicate::orientation(a, b, d, epsilon);
        int predecate_cda = predicate::orientation(c, d, a, epsilon);
        int predecate_cdb = predicate::orientation(c, d, b, epsilon);

        if ((predecate_abc != predecate_abd) and (predecate_cda != predecate_cdb)) return 0;

        if ((predecate_abc == 0) && on_segment(a, b, c, epsilon)) return 0;
        if ((predecate_abd == 0) && on_segment(a, b, d, epsilon)) return 0;
        if ((predecate_cda == 0) && on_segment(c, d, a, epsilon)) return 0;
        if ((predecate_cdb == 0) && on_segment(c, d, b, epsilon)) return 0;

        if ((predecate_abc == 0) && (predecate_abd == 0)) return 1;
        if ((predecate_cda == 0) && (predecate_cdb == 0)) return 1;

        return -1;
    }

    inline Point intersection_line(const Point& a, const Point& b, const Point& c, const Point& d, double epsilon = 1e-9)
    {
        double cross1 = a.x*b.y - a.y*b.x;
        double cross2 = c.x*d.y - c.y*d.x;

        double den = (a.x - b.x)*(c.y - d.y) - (a.y - b.y)*(c.x - d.x);

        if (std::abs(den) < epsilon)
            throw "Прямые плохо обусловленны";

        double x = (cross1*(c.x - d.x) - cross2*(a.x - b.x))/den;
        double y = (cross1*(c.y - d.y) - cross2*(a.y - b.y))/den;

        return Point(x, y);
    }
}


