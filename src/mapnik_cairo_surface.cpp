#include "utils.hpp"
#include "mapnik_cairo_surface.hpp"
using namespace v8;

Nan::Persistent<FunctionTemplate> CairoSurface::constructor;

void CairoSurface::Initialize(Handle<Object> target) {
    Nan::HandleScope scope;

    Local<FunctionTemplate> lcons = Nan::New<FunctionTemplate>(CairoSurface::New);
    lcons->InstanceTemplate()->SetInternalFieldCount(1);
    lcons->SetClassName(Nan::New("CairoSurface").ToLocalChecked());
    Nan::SetPrototypeMethod(lcons, "width", width);
    Nan::SetPrototypeMethod(lcons, "height", height);
    Nan::SetPrototypeMethod(lcons, "getData", getData);
    target->Set(Nan::New("CairoSurface").ToLocalChecked(), lcons->GetFunction());
    constructor.Reset(lcons);
}

CairoSurface::CairoSurface(std::string const& format, unsigned int width, unsigned int height) :
    Nan::ObjectWrap(),
    ss_(),
    width_(width),
    height_(height),
    format_(format)
{
}

CairoSurface::~CairoSurface()
{
}

NAN_METHOD(CairoSurface::New)
{
    Nan::HandleScope scope;
    if (!info.IsConstructCall())
    {
        Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
        return;
    }

    if (info[0]->IsExternal())
    {
        // Currently there is no C++ that executes this call
        /* LCOV_EXCL_START */
        Local<External> ext = info[0].As<External>();
        void* ptr = ext->Value();
        CairoSurface* im =  static_cast<CairoSurface*>(ptr);
        im->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
        return;
        /* LCOV_EXCL_END */
    }

    if (info.Length() == 3)
    {
        if (!info[0]->IsString())
        {
            Nan::ThrowTypeError("CairoSurface 'format' must be a string");
            return;
        }
        std::string format = TOSTR(info[0]);
        if (!info[1]->IsNumber() || !info[2]->IsNumber())
        {
            Nan::ThrowTypeError("CairoSurface 'width' and 'height' must be a integers");
            return;
        }
        CairoSurface* im = new CairoSurface(format, info[1]->IntegerValue(), info[2]->IntegerValue());
        im->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
        return;
    }
    else
    {
        Nan::ThrowError("CairoSurface requires three arguments: format, width, and height");
        return;
    }
    return;
}

NAN_METHOD(CairoSurface::width)
{
    Nan::HandleScope scope;

    CairoSurface* im = Nan::ObjectWrap::Unwrap<CairoSurface>(info.Holder());
    info.GetReturnValue().Set(Nan::New(im->width()));
}

NAN_METHOD(CairoSurface::height)
{
    Nan::HandleScope scope;

    CairoSurface* im = Nan::ObjectWrap::Unwrap<CairoSurface>(info.Holder());
    info.GetReturnValue().Set(Nan::New(im->height()));
}

NAN_METHOD(CairoSurface::getData)
{
    Nan::HandleScope scope;
    CairoSurface* surface = Nan::ObjectWrap::Unwrap<CairoSurface>(info.Holder());
    std::string s = surface->ss_.str();
    info.GetReturnValue().Set(Nan::NewBuffer((char*)s.data(), s.size()).ToLocalChecked());
}
