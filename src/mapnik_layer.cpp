#include "mapnik_layer.hpp"

#include "utils.hpp"                    // for TOSTR, ATTR, etc

#include "mapnik_datasource.hpp"
#include "mapnik_memory_datasource.hpp"

// mapnik
#include <mapnik/datasource.hpp>        // for datasource_ptr, datasource
#include <mapnik/memory_datasource.hpp> // for memory_datasource
#include <mapnik/layer.hpp>             // for layer
#include <mapnik/params.hpp>            // for parameters

// stl
#include <limits>

Nan::Persistent<FunctionTemplate> Layer::constructor;

void Layer::Initialize(Handle<Object> target) {

    Nan::HandleScope scope;

    Local<FunctionTemplate> lcons = Nan::New<FunctionTemplate>(Layer::New);
    lcons->InstanceTemplate()->SetInternalFieldCount(1);
    lcons->SetClassName(Nan::New("Layer").ToLocalChecked());

    // methods
    Nan::SetPrototypeMethod(lcons, "describe", describe);

    // properties
    ATTR(lcons, "name", get_prop, set_prop);
    ATTR(lcons, "active", get_prop, set_prop);
    ATTR(lcons, "srs", get_prop, set_prop);
    ATTR(lcons, "styles", get_prop, set_prop);
    ATTR(lcons, "datasource", get_prop, set_prop);
    ATTR(lcons, "minimum_scale_denominator", get_prop, set_prop);
    ATTR(lcons, "maximum_scale_denominator", get_prop, set_prop);
    ATTR(lcons, "queryable", get_prop, set_prop);
    ATTR(lcons, "clear_label_cache", get_prop, set_prop);

    target->Set(Nan::New("Layer").ToLocalChecked(),lcons->GetFunction());
    constructor.Reset(lcons);
}

Layer::Layer(std::string const& name):
    Nan::ObjectWrap(),
    layer_(std::make_shared<mapnik::layer>(name)) {}

Layer::Layer(std::string const& name, std::string const& srs):
    Nan::ObjectWrap(),
    layer_(std::make_shared<mapnik::layer>(name,srs)) {}

Layer::Layer():
    Nan::ObjectWrap(),
    layer_() {}


Layer::~Layer() {}

NAN_METHOD(Layer::New)
{
    Nan::HandleScope scope;

    if (!info.IsConstructCall()) {
        Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
        return;
    }

    if (info[0]->IsExternal())
    {
        Local<External> ext = info[0].As<External>();
        void* ptr = ext->Value();
        Layer* l =  static_cast<Layer*>(ptr);
        l->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
        return;
    }

    if (info.Length() == 1)
    {
        if (!info[0]->IsString())
        {
            Nan::ThrowTypeError("'name' must be a string");
            return;
        }
        Layer* l = new Layer(TOSTR(info[0]));
        l->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
        return;
    }
    else if (info.Length() == 2)
    {
        if (!info[0]->IsString() || !info[1]->IsString()) {
            Nan::ThrowTypeError("'name' and 'srs' must be a strings");
            return;
        }
        Layer* l = new Layer(TOSTR(info[0]),TOSTR(info[1]));
        l->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
        return;
    }
    else
    {
        Nan::ThrowTypeError("please provide Layer name and optional srs");
        return;
    }
    info.GetReturnValue().Set(info.This());
}

Local<Value> Layer::NewInstance(mapnik::layer const& lay_ref) {
    Nan::EscapableHandleScope scope;
    Layer* l = new Layer();
    // copy new mapnik::layer into the shared_ptr
    l->layer_ = std::make_shared<mapnik::layer>(lay_ref);
    Handle<Value> ext = Nan::New<External>(l);
    return scope.Escape(Nan::New(constructor)->GetFunction()->NewInstance(1, &ext));
}

NAN_GETTER(Layer::get_prop)
{
    Nan::HandleScope scope;
    Layer* l = Nan::ObjectWrap::Unwrap<Layer>(info.Holder());
    std::string a = TOSTR(property);
    if (a == "name")
        info.GetReturnValue().Set(Nan::New<String>(l->layer_->name()).ToLocalChecked());
    else if (a == "srs")
        info.GetReturnValue().Set(Nan::New<String>(l->layer_->srs()).ToLocalChecked());
    else if (a == "styles") {
        std::vector<std::string> const& style_names = l->layer_->styles();
        Local<Array> s = Nan::New<Array>(style_names.size());
        for (unsigned i = 0; i < style_names.size(); ++i)
        {
            s->Set(i, Nan::New<String>(style_names[i]).ToLocalChecked() );
        }
        info.GetReturnValue().Set(s);
    }
    else if (a == "datasource") {
        mapnik::datasource_ptr ds = l->layer_->datasource();
        if (ds)
        {
            mapnik::memory_datasource * mem_ptr = dynamic_cast<mapnik::memory_datasource*>(ds.get());
            if (mem_ptr)
            {
                info.GetReturnValue().Set(MemoryDatasource::NewInstance(ds));
            }
            else
            {
                info.GetReturnValue().Set(Datasource::NewInstance(ds));
            }
        }
        return;
    }
    else if (a == "minimum_scale_denominator") 
    {
        info.GetReturnValue().Set(Nan::New<Number>(l->layer_->minimum_scale_denominator()));   
    }
    else if (a == "maximum_scale_denominator") 
    {
        info.GetReturnValue().Set(Nan::New<Number>(l->layer_->maximum_scale_denominator()));   
    }
    else if (a == "queryable") 
    {
        info.GetReturnValue().Set(Nan::New<Boolean>(l->layer_->queryable()));   
    }
    else if (a == "clear_label_cache") 
    {
        info.GetReturnValue().Set(Nan::New<Boolean>(l->layer_->clear_label_cache()));   
    }
    else // if (a == "active") 
    {
        info.GetReturnValue().Set(Nan::New<Boolean>(l->layer_->active()));   
    }
}

NAN_SETTER(Layer::set_prop)
{
    Nan::HandleScope scope;
    Layer* l = Nan::ObjectWrap::Unwrap<Layer>(info.Holder());
    std::string a = TOSTR(property);
    if (a == "name")
    {
        if (!value->IsString()) {
            Nan::ThrowTypeError("'name' must be a string");
            return;
        } else {
            l->layer_->set_name(TOSTR(value));
        }
    }
    else if (a == "srs")
    {
        if (!value->IsString()) {
            Nan::ThrowTypeError("'srs' must be a string");
            return;
        } else {
            l->layer_->set_srs(TOSTR(value));
        }
    }
    else if (a == "styles")
    {
        if (!value->IsArray()) {
            Nan::ThrowTypeError("Must provide an array of style names");
            return;
        } else {
            Local<Array> arr = value.As<Array>();
            // todo - how to check if cast worked?
            unsigned int i = 0;
            unsigned int a_length = arr->Length();
            while (i < a_length) {
                l->layer_->add_style(TOSTR(arr->Get(i)));
                i++;
            }
        }
    }
    else if (a == "datasource")
    {
        Local<Object> obj = value.As<Object>();
        if (value->IsNull() || value->IsUndefined()) {
            Nan::ThrowTypeError("mapnik.Datasource, or mapnik.MemoryDatasource instance expected");
            return;
        } else {
            if (Nan::New(Datasource::constructor)->HasInstance(obj)) {
                Datasource *d = Nan::ObjectWrap::Unwrap<Datasource>(obj);
                l->layer_->set_datasource(d->get());
            }
            /*else if (Nan::New(JSDatasource::constructor)->HasInstance(obj))
            {
                JSDatasource *d = Nan::ObjectWrap::Unwrap<JSDatasource>(obj);
                l->layer_->set_datasource(d->get());
            }*/
            else if (Nan::New(MemoryDatasource::constructor)->HasInstance(obj))
            {
                MemoryDatasource *d = Nan::ObjectWrap::Unwrap<MemoryDatasource>(obj);
                l->layer_->set_datasource(d->get());
            }
            else
            {
                Nan::ThrowTypeError("mapnik.Datasource or mapnik.MemoryDatasource instance expected");
                return;
            }
        }
    }
    else if (a == "minimum_scale_denominator")
    {
        if (!value->IsNumber()) {
            Nan::ThrowTypeError("Must provide a number");
            return;
        }
        l->layer_->set_minimum_scale_denominator(value->NumberValue());
    }
    else if (a == "maximum_scale_denominator")
    {
        if (!value->IsNumber()) {
            Nan::ThrowTypeError("Must provide a number");
            return;
        }
        l->layer_->set_maximum_scale_denominator(value->NumberValue());
    }
    else if (a == "queryable")
    {
        if (!value->IsBoolean()) {
            Nan::ThrowTypeError("Must provide a boolean");
            return;
        }
        l->layer_->set_queryable(value->BooleanValue());
    }
    else if (a == "clear_label_cache")
    {
        if (!value->IsBoolean()) {
            Nan::ThrowTypeError("Must provide a boolean");
            return;
        }
        l->layer_->set_clear_label_cache(value->BooleanValue());
    }
    else if (a == "active")
    {
        if (!value->IsBoolean()) {
            Nan::ThrowTypeError("Must provide a boolean");
            return;
        }
        l->layer_->set_active(value->BooleanValue());
    }
}

NAN_METHOD(Layer::describe)
{
    Nan::HandleScope scope;

    Layer* l = Nan::ObjectWrap::Unwrap<Layer>(info.Holder());

    Local<Object> description = Nan::New<Object>();
    mapnik::layer const& layer = *l->layer_;
        
    description->Set(Nan::New("name").ToLocalChecked(), Nan::New<String>(layer.name()).ToLocalChecked());

    description->Set(Nan::New("srs").ToLocalChecked(), Nan::New<String>(layer.srs()).ToLocalChecked());

    description->Set(Nan::New("active").ToLocalChecked(), Nan::New<Boolean>(layer.active()));

    description->Set(Nan::New("clear_label_cache").ToLocalChecked(), Nan::New<Boolean>(layer.clear_label_cache()));

    description->Set(Nan::New("minimum_scale_denominator").ToLocalChecked(), Nan::New<Number>(layer.minimum_scale_denominator()));

    description->Set(Nan::New("maximum_scale_denominator").ToLocalChecked(), Nan::New<Number>(layer.maximum_scale_denominator()));

    description->Set(Nan::New("queryable").ToLocalChecked(), Nan::New<Boolean>(layer.queryable()));

    std::vector<std::string> const& style_names = layer.styles();
    Local<Array> s = Nan::New<Array>(style_names.size());
    for (unsigned i = 0; i < style_names.size(); ++i)
    {
        s->Set(i, Nan::New<String>(style_names[i]).ToLocalChecked() );
    }

    description->Set(Nan::New("styles").ToLocalChecked(), s );

    mapnik::datasource_ptr datasource = layer.datasource();
    Local<v8::Object> ds = Nan::New<Object>();
    description->Set(Nan::New("datasource").ToLocalChecked(), ds );
    if ( datasource )
    {
        mapnik::parameters::const_iterator it = datasource->params().begin();
        mapnik::parameters::const_iterator end = datasource->params().end();
        for (; it != end; ++it)
        {
            node_mapnik::params_to_object(ds, it->first, it->second);
        }
    }

    info.GetReturnValue().Set(description);
}
