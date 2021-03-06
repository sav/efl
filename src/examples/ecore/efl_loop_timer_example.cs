using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

class TestMain
{
    private efl.Loop loop;
    private int count;

    public TestMain(efl.Loop loop)
    {
        this.loop = loop;
        this.count = 0;
    }

    static void Main(string[] args)
    {
        efl.All.Init();

        efl.Loop loop = new efl.LoopConcrete();
        efl.loop.Timer timer = new efl.loop.TimerConcrete(loop);

        TestMain listener = new TestMain(loop);

        Console.WriteLine("Starting MainLoop");

        timer.interval_set(1.0);

        timer.TICK += listener.on_tick;
        timer.TICK += listener.another_callback;
        timer.TICK -= listener.another_callback;

        loop.begin();

        efl.All.Shutdown();
    }

    public void on_tick(object sender, EventArgs e)
    {
        Console.WriteLine("on_tick called on listener");

        if (count++ == 5)
            loop.quit(0);
    }

    public void another_callback(object sender, EventArgs e)
    {
        Console.WriteLine("Ooops. Should not have been called...");
    }
}


