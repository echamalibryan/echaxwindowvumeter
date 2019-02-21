#include "e.h"

/*
 * (copy of) Echa Corp and Echa Community
 *
 * Licensed under GPLv3 (GNU General Public License v3.0)
 */

EchaRender *er;

LinkData ld;
XServer x;
PulseAudio pa;
Gate gate;
Thread th;
EchaApp e;

void ex_th_xserver()
{
    x.Exec();
}

void ex_pa_context_begin(pa_context *context, void *lp)
{
    pa.context_begin(context);
}

void ex_pa_context_get_source_info(pa_context *context, const pa_source_info *si, int is_last, void *lp)
{
    pa.context_get_source_info(context, si, is_last);
}

void ex_pa_context_get_sink_info(pa_context *context, const pa_sink_info *si, int is_last, void *lp)
{
    pa.context_get_sink_info(context, si, is_last);
}

void ex_pa_context_get_server_info(pa_context *context, const pa_server_info *si, void *lp)
{
    pa.context_get_server_info(context, si);
}

void ex_pa_stream_state(pa_stream *stream, void *lp)
{
    pa.stream_state(stream);
}

void ex_pa_stream_read(pa_stream *stream, size_t l, void *lp)
{
    pa.stream_read(stream, l);
}

void ex_pa_update_timing_info(pa_stream *stream, int suc, void *lp)
{
    pa.update_timing_info(stream, suc);
}

void ex_pa_latency()
{
    pa.latency();
}

int main(int argc, char **argv)
{
    return e.Exec(argc, argv);
}

EchaApp::EchaApp()
{

}

Thread::Thread()
{

}

PulseAudio::PulseAudio()
{
    c_record = false;
}

XServer::XServer()
{

}

int EchaApp::Exec(int argc, char **argv)
{
    c_argc = argc;

    c_argv = argv;

    return pa.Exec();
}

int Thread::Exec()
{
    return 0;
}

void Thread::SetXServerThread()
{
    c_th_xserver = new std::thread(ex_th_xserver);
}

int PulseAudio::Exec()
{
    e_mode = pa.GetRecordStatus() ? E_RECORD : E_PLAYBACK;

    fprintf(stderr, "Start in %s mode\n", e_mode ? "record" : "playback");

    if(e.GetARGC() > 1)
    {
        char **cache = e.GetARGV();

        pa.SetDeviceName(cache[1]);
    }
    else
    {
        char *cache = getenv(e_mode ? "PULSE_SOURCE" : "PULSE_SINK");

        if(cache)
        {
            pa.SetDeviceName(cache);
        }
    }

    if(pa.GetDeviceName())
    {
        fprintf(stderr, "Using device %s\n", pa.GetDeviceName());
    }

    pa.SetMainLoop(pa_mainloop_new());

    if(!pa.GetMainLoop())
    {
        fprintf(stderr, "pa_loop failed\n");

        return -1;
    }

    pa_context *context_cache = pa_context_new(pa_mainloop_get_api(pa.GetMainLoop()), "X11 Pulse Audio");

    pa.SetContext(context_cache);

    if(!pa.GetContext())
    {
        fprintf(stderr, "context failed failed\n");

        return -1;
    }

    int rv;

    pa_context_set_state_callback(pa.GetContext(), ex_pa_context_begin, 0);

    pa_context_connect(pa.GetContext(), 0, PA_CONTEXT_NOAUTOSPAWN, 0);

    pa_mainloop_run(pa.GetMainLoop(), &rv);

    th.GetXServerThread()->join();

    return 0;
}

char * PulseAudio::SetDeviceName(char *scl)
{
    c_device_name = strdup(scl);

    return c_device_name;
}

pa_mainloop * PulseAudio::SetMainLoop(pa_mainloop *scl)
{
    c_loop = scl;

    return c_loop;
}

pa_context * PulseAudio::SetContext(pa_context *scl)
{
    c_context = scl;

    return c_context;
}

pa_stream * PulseAudio::SetStream(pa_stream *scl)
{
    c_stream = scl;

    return c_stream;
}

void PulseAudio::context_begin(pa_context *context)
{
    switch(pa_context_get_state(context))
    {
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_SETTING_NAME:
        case PA_CONTEXT_AUTHORIZING:
            {
                fprintf(stderr, "PA_CONTEXT_SETTING_NAME PA_CONTEXT_AUTHORIZING PA_CONTEXT_CONNECTING PA_CONTEXT_UNCONNECTED\n");
            }
            break;

        case PA_CONTEXT_READY:
            {
                if(c_device_name && e_mode == E_RECORD)
                {
                    pa_operation_unref(pa_context_get_source_info_by_name(context, c_device_name, ex_pa_context_get_source_info, 0));
                }
                else if(c_device_name && e_mode == E_PLAYBACK)
                {
                    pa_operation_unref(pa_context_get_sink_info_by_name(context, c_device_name, ex_pa_context_get_sink_info, 0));
                }
                else
                {
                    pa_operation_unref(pa_context_get_server_info(context, ex_pa_context_get_server_info, 0));
                }

            }
            break;

        case PA_CONTEXT_FAILED:
            {
                fprintf(stderr, "PA_CONTEXT_FAILED\n");
            }
            break;

        case PA_CONTEXT_TERMINATED:
            {
                fprintf(stderr, "PA_CONTEXT_TERMINATED\n");
            }
            break;

        default: break;
    }
}

void PulseAudio::context_get_source_info(pa_context *context, const pa_source_info *si, int is_last)
{
    fprintf(stderr, "context_get_source_info\n");

    if(is_last < 0)
        {
            fprintf(stderr, "Failed to get source information\n");

            return;
        }

        if(!si) return;

        pa.create_stream(si->name, si->description, si->sample_spec, si->channel_map);
}

void PulseAudio::context_get_sink_info(pa_context *context, const pa_sink_info *si, int is_last)
{
    fprintf(stderr, "context_get_sink_info\n");

    if(is_last < 0)
        {
            fprintf(stderr, "Failed to get sink information\n");

            return;
        }

        if(!si) return;

    pa.create_stream(si->monitor_source_name, si->description, si->sample_spec, si->channel_map);
}

void PulseAudio::context_get_server_info(pa_context *context, const pa_server_info *si)
{
    fprintf(stderr, "context_get_server_info\n");

    if(!si)
    {
        fprintf(stderr, "Failed to get server information\n");

        return;
    }

        if(e_mode == E_PLAYBACK)
        {
            if(!si->default_sink_name)
            {
                    fprintf(stderr, "No default sink set.\n");

                    return;
            }

            pa_operation_unref(pa_context_get_sink_info_by_name(context, si->default_sink_name, ex_pa_context_get_sink_info, 0));
        }
        else if(e_mode == E_RECORD)
        {
            if(!si->default_source_name)
            {
                    fprintf(stderr, "No default source set.\n");

                    return;
            }

            pa_operation_unref(pa_context_get_source_info_by_name(context, si->default_source_name, ex_pa_context_get_source_info, 0));
        }
        else
        {}
}

void PulseAudio::create_stream(const char *name, const char *description, const pa_sample_spec &ss, const pa_channel_map &cmap)
{
    fprintf(stderr, "create_stream\n");

    char t[256];
    pa_sample_spec nss;

    memcpy(&nss, &ss, sizeof(pa_sample_spec));

    free(c_device_name);

    c_device_name = strdup(name);

    free(c_device_desc);

    c_device_desc = strdup(description);

    nss.format = PA_SAMPLE_FLOAT32;

    fprintf(stderr, "Using sample format: %s\n", pa_sample_spec_snprint(t, sizeof(t), &nss));

    fprintf(stderr, "Channel is: %s\t%d\n", pa_channel_map_snprint(t, sizeof(t), &cmap), nss.channels);

    c_stream = pa_stream_new(c_context, "Pulse Audio Meter", &nss, &cmap);

    pa_stream_set_state_callback(c_stream, ex_pa_stream_state, 0);

    pa_stream_set_read_callback(c_stream, ex_pa_stream_read, 0);

    pa_stream_connect_record(c_stream, name, 0, (enum pa_stream_flags) 0);
}

void PulseAudio::stream_state(pa_stream *stream)
{
    switch(pa_stream_get_state(stream))
    {
        case PA_STREAM_CREATING:
        case PA_STREAM_UNCONNECTED:
            break;

        case PA_STREAM_READY:
            {
                pa_operation_unref(pa_stream_update_timing_info(stream, ex_pa_update_timing_info, 0));

                c_cmap = (pa_channel_map*)pa_stream_get_channel_map(stream);

                fprintf(stderr, "c_map->channels are: %d\n", c_cmap->channels);

                th.SetXServerThread();
            }
            break;

        case PA_STREAM_FAILED:
            {

            }
            break;

        case PA_STREAM_TERMINATED:
            {

            }
            break;

        default: break;
    }
}

void PulseAudio::stream_read(pa_stream *stream, size_t l)
{
    static void *p;

    if(pa_stream_peek(stream, (const void**)&p, &l) < 0)
    {
        fprintf(stderr, "pa_stream_peek failed: %s\n", pa_strerror(pa_context_errno(c_context)));

        return;
    }

    ld.SetData((float*)p);

    pa_stream_drop(stream);
}

void PulseAudio::update_timing_info(pa_stream *s, int suc)
{
    pa_usec_t t;
    int neg = 0;

    if(!suc || pa_stream_get_latency(s, &t, &neg) < 0)
    {
        fprintf(stderr, "Failed to get latency\n");

        return;
    }
}

void PulseAudio::latency()
{

}

int XServer::Exec()
{
    if(!c_w_size)
    {
        c_w_size = new size_t[pa.GetChannelSize()];

        memset(c_w_size, 0, pa.GetChannelSize() * sizeof(size_t));

        c_w_size[0] = 640;

        c_w_size[1] = 480;
    }

    XInitThreads();

    c_d = XOpenDisplay(0);

    if(!c_d)
    {
        fprintf(stderr, "X Display failed\n");

        return -1;
    }

    fprintf(stderr, "X Display succeded\n");

    c_r = DefaultRootWindow(c_d);

    c_numscr = DefaultScreen(c_d);

    c_swa.event_mask = ExposureMask | StructureNotifyMask;

    c_swa.border_pixel = XBlackPixel(c_d, c_numscr);

    c_swa.background_pixel = XWhitePixel(c_d, c_numscr);

    c_w = XCreateWindow(c_d, c_r, 0, 0, c_w_size[0], c_w_size[1], 1, 0, 0, 0, CWBackPixel | CWBorderPixel | CWEventMask, &c_swa);

    if(!c_w)
    {
        fprintf(stderr, "Window created failed\n");

        return -1;
    }

    Atom a = XInternAtom(c_d, "WM_DELETE_WINDOW", 0);

    XSetWMProtocols(c_d, c_w, &a, 1);

    XStoreName(c_d, c_w, "Echa X Window\0");

    XMapWindow(c_d, c_w);

    if(!c_cw)
    {
        c_cw = (Window*)malloc(pa.GetChannelSize() * sizeof(Window));

        for(size_t i = 0; i < pa.GetChannelSize(); ++i)
        {
            if(i == 1)
            {
                c_swa.background_pixel = XBlackPixel(c_d, c_numscr);

                c_cw[i] = XCreateWindow(c_d, c_w, c_w_size[0] / 2, 0, c_w_size[0] / 2, c_w_size[1], 1, 0, 0, 0, CWBackPixel | CWBorderPixel | CWEventMask, &c_swa);
            }
            else
            {
                c_cw[i] = XCreateWindow(c_d, c_w, 0, 0, c_w_size[0], c_w_size[1], 1, 0, 0, 0, CWBackPixel | CWBorderPixel | CWEventMask, &c_swa);
            }

            XMapWindow(c_d, c_cw[i]);
        }
    }

    c_val.line_style = LineSolid;

    c_val.line_width = 10;

    c_val.foreground = XBlackPixel(c_d, c_numscr);

    if(!c_pen)
    {
        c_pen = (GC*)malloc(pa.GetChannelSize() * sizeof(GC));

        for(size_t i = 0; i < pa.GetChannelSize(); ++i)
        {
            if(i == 1)
            {
                 c_val.foreground = XWhitePixel(c_d, c_numscr);
            }

            c_pen[i] = XCreateGC(c_d, c_cw[i], GCLineStyle | GCLineWidth | GCForeground, &c_val);
        }
    }

    Atom wmsize = XInternAtom(c_d, "WM_NORMAL_HINTS\0", 0);

    XSizeHints xsh = { };

    xsh.flags = PSize | PMaxSize | PMinSize;
    xsh.max_width = c_w_size[0];
    xsh.min_width = c_w_size[0];
    xsh.max_height = c_w_size[1];
    xsh.min_height = c_w_size[1];

    XSetSizeHints(c_d, c_w, &xsh, wmsize);

    XFlush(c_d);

    size_t x, y, width, height;

    while(1)
    {
        if(XPending(c_d) > 0)
        {
            XNextEvent(c_d, &c_e);

            switch(c_e.type)
            {
                case Expose:
                    {

                    }
                    break;

                case ConfigureNotify:
                    {
                        if(c_e.xany.window != c_w) break;

                        for(size_t i = 0; i < pa.GetChannelSize(); ++i)
                        {
                            x = c_e.xconfigure.x;
                            y = c_e.xconfigure.y;
                            width = c_e.xconfigure.width;
                            height = c_e.xconfigure.height;

                            if(i == 0)
                            {
                                XMoveResizeWindow(c_d, c_cw[i], 0, 0, width / 2, height);
                            }

                            if(i == 1)
                            {
                                XMoveResizeWindow(c_d, c_cw[i], width / 2, 0, width / 2, height);
                            }
                        }


                    };
                    break;

                case ClientMessage:
                    {
                        XCloseDisplay(c_d);

                        pa_mainloop_quit(pa.GetMainLoop(), 1);
                    }
                    return 1;

                default: break;
            }
        }

        for(size_t i = 0; i < pa.GetChannelSize(); ++i)
        {
            c_level[i] = (long)(500 * fabs(ld.GetData()[i])) + 1;
        }

        static long *cache_level;

        if(!cache_level)
        {
            cache_level = (long*)malloc(pa.GetChannelSize() * sizeof(long));

            memset(cache_level, 0, pa.GetChannelSize() * sizeof(long));
        }

        for(size_t i = 0; i < pa.GetChannelSize(); ++i)
        {
            if(cache_level[i] < c_level[i])
            {
                cache_level[i] = c_level[i];
            }

            if(cache_level[i] < 2)
            {
                cache_level[i] = 2;
            }

            XClearWindow(c_d, c_cw[i]);

            XDrawRectangle(c_d, c_cw[i], c_pen[i], 10, (height - 10) - cache_level[i], (width / 2) - (c_val.line_width * 2), cache_level[i]);

            if(cache_level[i] > 0) --cache_level[i];
        }

        std::this_thread::sleep_for(std::chrono::microseconds(2048));
    }

    return 0;
}

float * LinkData::SetData(float *scl)
{
    if(pa.GetChannelSize() > MaxChannel)
    {
        fprintf(stderr, "Limit channel size data\n");

        return 0;
    }

    for(size_t i = 0; i < pa.GetChannelSize(); ++i)
    {
        c_data[i] = scl[i];
    }

    return c_data;
}
