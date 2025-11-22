#include "transform.h"
#include "json11.hpp"

Transform* Transform::Factory(const Json &json)
{
    if (!json["method"].is_string())
        return NULL;

    Transform *tr = NULL;

    if (json["method"].string_value() == "translate")
        tr = new TranslateLRF(json);
    else if (json["method"].string_value() == "rotate")
        tr = new RotateLRF(json);
    else if (json["method"].string_value() == "reflect")
        tr = new ReflectLRF(json);
    else
        return NULL;

    if (tr->fValid)
        return tr;
    else {
        delete tr;
        return NULL;
    }
}

// ============== Translate ===============

TranslateLRF::TranslateLRF(double dx, double dy)
{
    this->dx = dx;
    this->dy = dy;
    fValid = true;
}

TranslateLRF::TranslateLRF(const Json &json)
{
    if (!json["dx"].is_number() || !json["dy"].is_number())
        return;
    dx = json["dx"].number_value();
    dy = json["dy"].number_value();
    fValid = true;
}

void TranslateLRF::DoTransform(double *x, double *y, double *z) const
{
    *x += dx;
    *y += dy;
}

void TranslateLRF::DoInvTransform(double *x, double *y, double *z) const
{
    *x -= dx;
    *y -= dy;
}

void TranslateLRF::ToJsonObject(Json_object &json) const
{
    json["method"] = "translate";
    json["dx"] = dx;
    json["dy"] = dy;
}

// ============== Rotate ===============

RotateLRF::RotateLRF(double phi)
{
    this->phi = phi;
    Init();
    fValid = true;
}

RotateLRF::RotateLRF(const Json &json)
{
    if (!json["phi"].is_number())
        return;
    phi = json["phi"].number_value();
    Init();
    fValid = true;
}

void RotateLRF::Init()
{
    A << cos(phi), -sin(phi),
         sin(phi),  cos(phi);
    B << cos(phi),  sin(phi),
        -sin(phi),  cos(phi);
}

void RotateLRF::DoTransform(double *x, double *y, double *z) const
{
    Vector2d pos_world(*x, *y), pos_local;
//    pos_world << *x << *y;
    pos_local = A * pos_world;
    *x = pos_local(0);
    *y = pos_local(1);
}

void RotateLRF::DoInvTransform(double *x, double *y, double *z) const
{
    Vector2d pos_world, pos_local(*x, *y);
//    pos_local << *x << *y;
    pos_world = B * pos_local;
    *x = pos_world(0);
    *y = pos_world(1);
}

void RotateLRF::ToJsonObject(Json_object &json) const
{
    json["method"] = "rotate";
    json["phi"] = phi;
}

// ============== Reflect ===============

ReflectLRF::ReflectLRF(double phi)
{
    this->phi = phi;
    Init();
    fValid = true;
}

ReflectLRF::ReflectLRF(const Json &json)
{
    if (!json["phi"].is_number())
        return;
    phi = json["phi"].number_value();
    Init();
    fValid = true;
}

void ReflectLRF::Init()
{
    A << cos(phi*2),  sin(phi*2),
         sin(phi*2), -cos(phi*2);
}

void ReflectLRF::DoTransform(double *x, double *y, double *z) const
{
    Vector2d pos_world(*x, *y), pos_local;
//    pos_world << *x << *y;
    pos_local = A * pos_world;
    *x = pos_local(0);
    *y = pos_local(1);
}

void ReflectLRF::DoInvTransform(double *x, double *y, double *z) const
{
    Vector2d pos_world, pos_local(*x, *y);
//    pos_local << *x << *y;
    pos_world = A * pos_local;
    *x = pos_world(0);
    *y = pos_world(1);
}

void ReflectLRF::ToJsonObject(Json_object &json) const
{
    json["method"] = "reflect";
    json["phi"] = phi;
}





