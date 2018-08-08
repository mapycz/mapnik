#include <mapnik/grid_index.hpp>
#include <mapnik/value_types.hpp>

#include <unordered_set>
#include <cmath>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <unicode/unistr.h>
#pragma GCC diagnostic pop

namespace mapnik {

template <class T>
grid_index<T>::grid_index(const double width_, const double height_, const int16_t cellSize_) :
    width(width_),
    height(height_),
    xCellCount(std::ceil(width_ / cellSize_)),
    yCellCount(std::ceil(height_ / cellSize_)),
    xScale(xCellCount / width_),
    yScale(yCellCount / height_)
    {
        boxCells.resize(xCellCount * yCellCount);
    }

template <class T>
void grid_index<T>::insert(T&& t, const BBox& bbox) {
    size_t uid = boxElements.size();

    auto cx1 = convertToXCellCoord(bbox.minx());
    auto cy1 = convertToYCellCoord(bbox.miny());
    auto cx2 = convertToXCellCoord(bbox.maxx());
    auto cy2 = convertToYCellCoord(bbox.maxy());

    int16_t x, y, cellIndex;
    for (x = cx1; x <= cx2; ++x) {
        for (y = cy1; y <= cy2; ++y) {
            cellIndex = xCellCount * y + x;
            boxCells[cellIndex].push_back(uid);
        }
    }

    boxElements.emplace_back(t, bbox);
}

template <class T>
void grid_index<T>::clear() {
    boxElements.clear();
    boxCells.clear();
}

template <class T>
std::vector<T> grid_index<T>::query(const BBox& queryBBox) const {
    std::vector<T> result;
    query(queryBBox, [&](const T& t, const BBox&) -> bool {
        result.push_back(t);
        return false;
    });
    return result;
}

template <class T>
std::vector<std::pair<T, typename grid_index<T>::BBox>> grid_index<T>::queryWithBoxes(const BBox& queryBBox) const {
    std::vector<std::pair<T, BBox>> result;
    query(queryBBox, [&](const T& t, const BBox& bbox) -> bool {
        result.push_back(std::make_pair(t, bbox));
        return false;
    });
    return result;
}

template <class T>
bool grid_index<T>::hitTest(const BBox& queryBBox) const {
    bool hit = false;
    query(queryBBox, [&](const T&, const BBox&) -> bool {
        hit = true;
        return true;
    });
    return hit;
}

template <class T>
bool grid_index<T>::noIntersection(const BBox& queryBBox) const {
    return queryBBox.maxx() < 0 || queryBBox.minx() >= width ||
    queryBBox.maxy() < 0 || queryBBox.miny() >= height;
}

template <class T>
bool grid_index<T>::completeIntersection(const BBox& queryBBox) const {
    return queryBBox.minx() <= 0 && queryBBox.miny() <= 0 && width <=
    queryBBox.maxx() && height <= queryBBox.maxy();
}

template <class T>
void grid_index<T>::query(const BBox& queryBBox, std::function<bool (const T&, const BBox&)> resultFn) const {
    std::unordered_set<size_t> seenBoxes;
    std::unordered_set<size_t> seenCircles;
    
    if (noIntersection(queryBBox)) {
        return;
    } else if (completeIntersection(queryBBox)) {
        for (auto& element : boxElements) {
            if (resultFn(element.first, element.second)) {
                return;
            }
        }
        return;
    }

    auto cx1 = convertToXCellCoord(queryBBox.minx());
    auto cy1 = convertToYCellCoord(queryBBox.miny());
    auto cx2 = convertToXCellCoord(queryBBox.maxx());
    auto cy2 = convertToYCellCoord(queryBBox.maxy());

    int16_t x, y, cellIndex;
    for (x = cx1; x <= cx2; ++x) {
        for (y = cy1; y <= cy2; ++y) {
            cellIndex = xCellCount * y + x;
            // Look up other boxes
            for (auto uid : boxCells[cellIndex]) {
                if (seenBoxes.count(uid) == 0) {
                    seenBoxes.insert(uid);

                    auto& pair = boxElements.at(uid);
                    auto& bbox = pair.second;
                    if (boxesCollide(queryBBox, bbox)) {
                        if (resultFn(pair.first, bbox)) {
                            return;
                        }
                    }
                }
            }
        }
    }
}

template <class T>
int16_t grid_index<T>::convertToXCellCoord(const double x) const {
    return std::max(0.0, std::min(xCellCount - 1.0, std::floor(x * xScale)));
}

template <class T>
int16_t grid_index<T>::convertToYCellCoord(const double y) const {
    return std::max(0.0, std::min(yCellCount - 1.0, std::floor(y * yScale)));
}

template <class T>
bool grid_index<T>::boxesCollide(const BBox& first, const BBox& second) const {
	return first.minx() <= second.maxx() &&
           first.miny() <= second.maxy() &&
           first.maxx() >= second.minx() &&
           first.maxy() >= second.miny();
}

template <class T>
bool grid_index<T>::empty() const {
    return boxElements.empty();
}

template <class T>
std::size_t grid_index<T>::size() const {
    return boxElements.size();
}


template class grid_index<mapnik::value_unicode_string>;
template class grid_index<int16_t>;

} // namespace mapnik
