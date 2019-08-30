#pragma once
#include "SkTools.h"
class SkCamera
{
private:
    float fov = 0.0f;
    float znear = 0.0f;
    float zfar = 0.0f;
    float walkSpeed = 0.005f;
    float turnSpeed = 1.0f;
    void Reortho()
    {
        this->front.Normalize();
        this->right = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(this->front, this->up));
        this->up = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(this->right, this->front));
    }
    void UpdateViewMatrix()
    {
        view = Matrix::CreateLookAt(this->pos, this->pos + this->front, this->up);
    }

public:
    Vector3 pos = {0.0f, 0.0f, 0.0f};
    Vector3 right = {1.0f, 0.0f, 0.0f};
    Vector3 up = {0.0f, 1.0f, 0.0f};
    Vector3 front = {0.0f, 0.0f, 1.0f};
    float upTime = 0.0f;
    Matrix proj;
    Matrix view;
    struct
    {
        bool left = false;
        bool right = false;
        bool front = false;
        bool back = false;
    } keys;

    Vector3 GetPosition() const { return pos; }
    void SetPostion(float x, float y, float z)
    {
        pos.x = x;
        pos.y = y;
        pos.z = z;
        UpdateViewMatrix();
    }
    void SetPostion(const Vector3 &v)
    {
        pos = v;
        UpdateViewMatrix();
    }
    void SetLens(float fov, float aspect, float zn, float zf)
    {
        this->fov = fov;
        this->znear = zn;
        this->zfar = zf;
        this->proj = Matrix::CreatePerspectiveFieldOfView(fov, aspect, zn, zf);
    }
    void SetLookAt(const Vector3 &pos, const Vector3 &target, const Vector3 &up)
    {
        this->pos = pos;
        this->front = DirectX::XMVector3Normalize(target - pos);
        this->right = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(this->front, up));
        this->up = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(this->right, this->front));
        this->view = Matrix::CreateLookAt(this->pos, target, this->up);
    }

    void Strafe(float d)
    {
        this->pos += d * this->right;
        this->UpdateViewMatrix();
    }
    void Walk(float d)
    {
        this->pos += d * this->front;
        this->UpdateViewMatrix();
    }
    void Walking(float delta)
    {
        if (keys.front | keys.back | keys.left | keys.right)
        {
            upTime = GetMilliTime();
            Vector3 dir = (float)(keys.front - keys.back) * front + (float)(keys.right - keys.left) * right;
            dir.Normalize();
            this->pos += delta * walkSpeed * dir;
            this->UpdateViewMatrix();
        }
    }
    void Turn(float angleY, float angleX)
    {
        upTime = GetMilliTime();
        Matrix R = Matrix::CreateRotationY(turnSpeed * angleY);
        R = R * DirectX::XMMatrixRotationAxis(this->right, turnSpeed * angleX);
        this->right = DirectX::XMVector3TransformNormal(this->right, R);
        this->up = DirectX::XMVector3TransformNormal(this->up, R);
        this->front = DirectX::XMVector3TransformNormal(this->front, R);
        this->Reortho();
        this->UpdateViewMatrix();
    }
    void Pitch(float angle)
    {
        upTime = GetMilliTime();
        Matrix R = DirectX::XMMatrixRotationAxis(this->right, turnSpeed * angle);
        this->up = DirectX::XMVector3TransformNormal(this->up, R);
        this->front = DirectX::XMVector3TransformNormal(this->front, R);
        this->Reortho();
        this->UpdateViewMatrix();
    }
    void Turn(float angle)
    {
        upTime = GetMilliTime();
        Matrix R = Matrix::CreateRotationY(turnSpeed * angle);
        this->right = DirectX::XMVector3TransformNormal(this->right, R);
        this->up = DirectX::XMVector3TransformNormal(this->up, R);
        this->front = DirectX::XMVector3TransformNormal(this->front, R);
        this->Reortho();
        this->UpdateViewMatrix();
    }
    SkCamera(/* args */) {}
    ~SkCamera() {}
};
