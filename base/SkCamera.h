#pragma once
#include "SkTools.h"
class SkCamera
{
private:
    float fov = 0.0f;
    float znear = 0.0f;
    float zfar = 0.0f;
    void UpdateViewMatrix()
    {
    }
    Vector3 pos = {0.0f, 0.0f, 0.0f};
    Vector3 right = {1.0f, 0.0f, 0.0f};
    Vector3 up = {0.0f, 1.0f, 0.0f};
    Vector3 front = {0.0f, 0.0f, 1.0f};

public:
    Matrix proj;
    Matrix view;

    Vector3 GetPosition() const { return pos; }
    void SetPostion(float x, float y, float z);
    void SetPostion(const Vector3 &v);
    void SetLens(float fov, float aspect, float zn, float zf)
    {
        this->fov=fov;
        this->znear=zn;
        this->zfar=zf;
        proj=Matrix::CreatePerspectiveFieldOfView(fov,aspect,zn,zf);
    }
    void SetLookAt(const Vector3 &pos, const Vector3 &target, const Vector3 &up)
    {
        this->pos=pos;
        this->front= DirectX::XMVector3Normalize(target-pos);
        this->right=DirectX::XMVector3Cross(this->front,up);
        this->up=DirectX::XMVector3Cross(this->right,this->front);
        view=Matrix::CreateLookAt(this->pos,target,this->up);
    }

    void Strafe(float d)
    {
        this->pos+=d*this->right;
        this->UpdateViewMatrix();
    }
    void walk(float d)
    {
        this->pos+=d*this->front;
        this->UpdateViewMatrix();
    }
    void Pitch(float angle);
    void Turn(float angle);
    SkCamera(/* args */) {}
    ~SkCamera() {}
};
