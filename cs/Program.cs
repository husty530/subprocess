using System.Text.Json;
using OpenCvSharp;

// register cancellation behavior
using var cts = new CancellationTokenSource();
cts.Token.Register(() => Console.WriteLine("cancellation invoked."));
Console.CancelKeyPress += (s, e) => cts.Cancel();

// prepare video
var w = 1920;
var h = 1080;
using var frame = new Mat(h, w, MatType.CV_8UC3);
using var cap = new VideoCapture() { FrameWidth = w, FrameHeight = h, Fps = 30 };
var f = cap.Open(Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "../../../../sample.mp4"));

// create SubProcess instance
var exe = "/** absolute path **/python.exe";
var py = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "../../../../py/main.py");
using var process = new SubProcess(exe, py, w, h);

// loop
try
{
    while (cap.Read(frame) && !cts.IsCancellationRequested)
    {
        unsafe { process.BinaryWriter.Write(new ReadOnlySpan<byte>(frame.DataPointer, w * h * 3)); }
        var json = process.StreamReader.ReadLine();
        var results = JsonSerializer.Deserialize<DnnResult[]>(json);
        // unpack the results.
        foreach (var r in results.Select(r => r.AsBox()))
        {
            Cv2.Rectangle(frame, r.Value, Scalar.Black, 2);
            Cv2.PutText(frame, r.Key, r.Value.TopLeft, HersheyFonts.HersheyPlain, 2, Scalar.Black, 2);
        }
        Cv2.ImShow("FRAME", frame);
        Cv2.WaitKey(1);
    }
    Console.WriteLine("Successfully completed.");
}
catch (Exception e)
{
    Console.WriteLine(e.ToString());
}
finally
{
    Console.WriteLine("Exit process.");
}