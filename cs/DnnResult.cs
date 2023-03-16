using OpenCvSharp;

public record class DnnResult(string label, double[][] coordinates)
{

    public override string ToString()
    {
        return $"{label}:[{string.Join(',', coordinates.Select(p => $"({p[0]:f1} {p[1]:f1})"))}]";
    }

    public KeyValuePair<string, Rect> AsBox()
    {
        var p0 = new Point(coordinates[0][0], coordinates[0][1]);
        var size = new Size(coordinates[3][0] - p0.X, coordinates[3][1] - p0.Y);
        return new(label, new(p0, size));
    }

    public KeyValuePair<string, Point[]> AsPolygon()
    {
        return new(label, coordinates.Select(p => new Point(p[0], p[1])).ToArray());
    }

}