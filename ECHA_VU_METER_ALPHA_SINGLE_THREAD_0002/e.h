#ifndef E_H
#define E_H

/*
 * (copy of) Echa Corp and Echa Community
 *
 * Licensed under GPLv3 (GNU General Public License v3.0)
 */

#define NDEBUG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <pulse/pulseaudio.h>
#include <pulse/mainloop.h>

#define SampleSize 4
#define MaxChannel 256

class EchaRender;

class LinkData;
class XServer;
class PulseAudio;
class Gate;
class Thread;
class EchaApp;

extern EchaRender *er;

extern LinkData ld;
extern XServer x;
extern PulseAudio pa;
extern Gate gate;
extern Thread th;
extern EchaApp e;

enum
{
    E_PLAYBACK,
    E_RECORD
} e_mode = E_PLAYBACK;

class EchaRender
{
    private:
        Display *c_d;
        Window c_w;
        GC c_pen;

    public:
        EchaRender();

        int Exec();

        void SetData();
};

class LinkData
{
    private:
        float c_data[MaxChannel];

    public:
        LinkData(){ fprintf(stderr, "LinkData\n"); }

        float * SetData(float *scl);

        float * GetData() { return c_data; }
};

class XServer
{
    private:
        long c_level[MaxChannel];
        size_t *c_w_size;
        Display *c_d;
        Window c_r;
        Window c_w;
        Drawable *c_cw;
        XSetWindowAttributes c_swa;
        XEvent c_e;
        size_t c_numscr;
        GC *c_pen;
        XGCValues c_val;

    public:
        XServer();

        int Exec();
};

class PulseAudio
{
    private:
        bool c_record;
        char *c_device_name;
        char *c_device_desc;
        pa_mainloop *c_loop;
        pa_context *c_context;
        pa_stream *c_stream;
        pa_channel_map *c_cmap;

        float *data;

    public:
        PulseAudio();

        int Exec();

        char * SetDeviceName(char *scl);
        pa_mainloop * SetMainLoop(pa_mainloop *scl);
        pa_context * SetContext(pa_context *scl);
        pa_stream * SetStream(pa_stream *scl);

        bool GetRecordStatus() { return c_record; }
        char * GetDeviceName() { return c_device_name; }
        pa_mainloop * GetMainLoop() { return c_loop; }
        pa_context * GetContext() { return c_context; }
        pa_stream * GetStream() { return c_stream; }
        size_t GetChannelSize() { return c_cmap->channels; }

        void context_begin(pa_context *context);
        void context_get_source_info(pa_context *context, const pa_source_info *si, int is_last);
        void context_get_sink_info(pa_context *context, const pa_sink_info *si, int is_last);
        void context_get_server_info(pa_context *context, const pa_server_info *si);
        void create_stream(const char *name, const char *description, const pa_sample_spec &ss, const pa_channel_map &cmap);
        void stream_state(pa_stream *stream);
        void stream_read(pa_stream *stream, size_t l);
        void update_timing_info(pa_stream *s, int suc);
        void latency();
};

class Gate
{
    private:
        int c_block;

    public:
        Gate(){ fprintf(stderr, "Gate\n"); }

        void SetBlockOn(){ c_block = 1; }
        void SetBlockOff(){ c_block = 0; }
        int GetBlockStatus(){ return c_block; }
};

class Thread
{
    private:
        std::thread *c_th_xserver;

    public:
        Thread();

        int Exec();

        void SetXServerThread();
        std::thread * GetXServerThread(){ return c_th_xserver; }
};

class EchaApp
{
    private:
        int c_argc;
        char **c_argv;

    public:
        EchaApp();

        int Exec(int argc, char **argv);

        int GetARGC() { return c_argc; }
        char ** GetARGV() { return c_argv; }
};

#endif // E_H
