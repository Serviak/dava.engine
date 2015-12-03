import QtQuick 2.3

Item
{
    property var container;
    property real xScale: 1;
    property real yScale: -1;
    property point origin: Qt.point(container ? container.width * 0.5 : 0,
                                    container ? container.height * 0.5 : 0)

    function scale(scaleFactor)
    {
        xScale *= scaleFactor;
        yScale *= scaleFactor;

        xScale = Math.max(xScale, 0.01);
        yScale = Math.min(yScale, -0.01);
    }

    function transformX(val)
    {
        return val * xScale + origin.x;
    }

    function transformY(val)
    {
        return val * yScale + origin.y;
    }

    function transform(pointVal)
    {
        return Qt.point(transformX(pointVal.x), transformY(pointVal.y));
    }

    function inverseTransform(pointVal)
    {
        return Qt.point((pointVal.x - origin.x) / xScale, (pointVal.y - origin.y) / yScale);
    }

    function shift(delta)
    {
        origin.x += delta.x;
        origin.y += delta.y;
    }
}