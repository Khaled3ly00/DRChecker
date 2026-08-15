#include "drcheck/spatial/QuadTree.h"

#include <stdexcept>
#include <utility>


namespace drcheck::spatial {
// Public constructor for the root node.
// Delegates to the private constructor with root depth = 0.
QuadTree::QuadTree(geometry::BoundingBox boundary, std::size_t capacity, std::size_t maxDepth) 
    : QuadTree(std::move(boundary), capacity, maxDepth, 0)
{
}
// Private constructor used internally to create child nodes.
QuadTree::QuadTree(geometry::BoundingBox boundary, std::size_t capacity, std::size_t maxDepth, std::size_t depth)
    : boundary(std::move(boundary)), capacity(capacity), maxDepth(maxDepth), depth(depth)
{
    if (capacity == 0) {
        throw std::invalid_argument("QuadTree capacity must be greater than zero");
    }
    // Child unique_ptr members are default-initialized to nullptr.
}

void QuadTree::subdivide() {
    // Split the current node at the midpoint of its boundary.
    const double midX = (boundary.getMinX() + boundary.getMaxX()) / 2.0;
    const double midY = (boundary.getMinY() + boundary.getMaxY()) / 2.0;

    // Cannot use make_unique as QuadTree uses the private constructor which can't be accessed by std::make_unique
    northWest = std::unique_ptr<QuadTree> (new QuadTree(
        geometry::BoundingBox(boundary.getMinX(), midY, midX, boundary.getMaxY()),
        capacity, maxDepth, depth + 1));

    northEast = std::unique_ptr<QuadTree> (new QuadTree(
        geometry::BoundingBox( midX, midY, boundary.getMaxX(), boundary.getMaxY()),
        capacity, maxDepth, depth + 1));

    southWest = std::unique_ptr<QuadTree> (new QuadTree(
        geometry::BoundingBox(boundary.getMinX(), boundary.getMinY(), midX, midY),
        capacity, maxDepth, depth + 1));

    southEast = std::unique_ptr<QuadTree>( new QuadTree(
        geometry::BoundingBox(midX,boundary.getMinY(),boundary.getMaxX(),midY), 
        capacity, maxDepth, depth + 1));
    
    // Existing shapes may now be able to move into one of
    // the newly created child nodes.
    std::vector<const domain::Shape*> remainingShapes;
    remainingShapes.reserve(shapes.size());

    // Move a shape into a child only when its bounding box
    // fits completely inside that single child.
    // Shapes crossing child boundaries remain in this node.
    for (const domain::Shape* shape :shapes)
    {
        const geometry::BoundingBox shapeBox = shape->getPolygon().getBoundingBox();

        if (QuadTree* child = getContainingChild(shapeBox))
        {
            child->insert(*shape);
        }
        else
        {
            remainingShapes.push_back(shape);
        }
    }
    // Replace the node's shape list with only the shapes
    // that could not be moved into a child.
    shapes = std::move(remainingShapes);
}

bool QuadTree::isSubdivided() const
{
    // All four children are created together in subdivide(),
    // so checking one child is enough to determine whether
    // this node has been subdivided.
    return northWest != nullptr;
}

// Returns the child whose boundary completely contains the given box.
// The returned raw pointer does not transfer ownership by using .get();
// the child remains owned by its std::unique_ptr.
QuadTree* QuadTree::getContainingChild(const geometry::BoundingBox& box)
{
    if (!isSubdivided()) {
        return nullptr;
    }

    if (northWest->boundary.contains(box)) {
        return northWest.get();
    }

    if (northEast->boundary.contains(box)) {
        return northEast.get();
    }

    if (southWest->boundary.contains(box)) {
        return southWest.get();
    }

    if (southEast->boundary.contains(box)) {
        return southEast.get();
    }
    // The box crosses one or more child boundaries,
    // so it must remain in the current node.
    return nullptr;
}

void QuadTree::insert(const domain::Shape& shape) {
    // Get shape boundary box
    const geometry::BoundingBox shapeBox = shape.getPolygon().getBoundingBox();
    // Check if the shape bounding box is inside root (QuadTree boundary)
    if (!boundary.contains(shapeBox)) {
        throw std::invalid_argument("Shape lies outside QuadTree boundary");
    }
    // If this node already has children, try to push
    // the shape deeper into the tree.
    if (isSubdivided())
    {
        // If the shape bounding box is within any child bounding box
        // Insert the shape
        QuadTree* child = getContainingChild(shapeBox);
        if (child != nullptr)
        {
            // Recrusive to check if this child is already subdivided again
            child->insert(shape);
        }
        else
        {
            // The shape crosses child boundaries and cannot be stored
            // completely inside a single child, so keep it in this node.
            shapes.push_back(&shape);
        }

        return;
    }
    // If this leaf node still has capacity, store the shape here.
    // Also stop subdividing once the maximum depth is reached.
    if (shapes.size() < capacity || depth >= maxDepth)
    {
        shapes.push_back(&shape);
        return;
    }
    // If capacity is maxed and still not reached max depth
    // Subdivide and distribute shapes in parent to relevant childs
    subdivide();

    // Then add new shape to relevant child
    if (QuadTree* child = getContainingChild(shapeBox))
    {
        child->insert(shape);
    }
    else
    {
        shapes.push_back(&shape);
    }

}
std::vector<const domain::Shape*> QuadTree::query(const geometry::BoundingBox& region) const
{
    std::vector<const domain::Shape*> results;

    queryRecursive(region, results);

    return results;
}
void QuadTree::queryRecursive(const geometry::BoundingBox& region, std::vector<const domain::Shape*>& results) const
{
    // If this node's entire boundary does not overlap the query region,
    // none of the shapes stored in this node or its descendants
    // can be relevant, so stop searching this branch.
    if (!boundary.overlaps(region)) {
        return;
    }
    // Check shapes stored directly in this node.
    // These may include shapes that could not be placed into a child
    // because their bounding boxes crossed child boundaries.
    for (const domain::Shape* shape :shapes)
    {
        const geometry::BoundingBox shapeBox = shape->getPolygon().getBoundingBox();

        if (shapeBox.overlaps(region)) {
            results.push_back(shape);
        }
    }
    // If this node has no children, there is nowhere deeper to search.
    if (!isSubdivided()) {
        return;
    }
    // Recursively search the children.
    // Each child performs its own boundary-overlap test at the
    // beginning of queryRecursive(), so branches that do not overlap
    // the query region will immediately return.
    northWest->queryRecursive(region, results);
    northEast->queryRecursive(region, results);
    southWest->queryRecursive(region, results);
    southEast->queryRecursive(region, results);
}
}