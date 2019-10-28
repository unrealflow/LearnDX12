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

public:
    void UpdateView()
    {
        if (true == needUpdate)
        {
            view = Matrix::CreateLookAt(this->pos, this->pos + this->front, this->up);
            upTime = GetMilliTime();
            needUpdate = false;
        }
    }
    enum CameraType
    {
        LookAt,
        FirstPerson
    };
    CameraType type=CameraType::LookAt;
    Vector3 pos = {0.0f, 0.0f, 0.0f};
    Vector3 right = {1.0f, 0.0f, 0.0f};
    Vector3 up = {0.0f, 1.0f, 0.0f};
    Vector3 front = {0.0f, 0.0f, 1.0f};
    float distance=1.0;
    float upTime = 0.0f;
    bool needUpdate = false;
    Matrix proj;
    Matrix view;
    struct
    {
        bool left = false;
        bool right = false;
        bool front = false;
        bool back = false;
        bool up = false;
        bool down = false;
    } keys;

    Vector3 GetPosition() const { return pos; }
    Vector3 GetTarget() const
    {
        return pos+distance*front;
    }
    void SetPostion(float x, float y, float z)
    {
        pos.x = x;
        pos.y = y;
        pos.z = z;
        needUpdate = true;
    }
    void SetPostion(const Vector3 &v)
    {
        pos = v;
        needUpdate = true;
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
        this->distance=(target-pos).Length();
        this->front = DirectX::XMVector3Normalize(target - pos);
        this->right = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(this->front, up));
        this->up = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(this->right, this->front));
        this->view = Matrix::CreateLookAt(this->pos, target, this->up);
    }
    void Lift(float d)
    {
        this->pos += d * this->up;
        this->needUpdate = true;
    }
    void Strafe(float d)
    {
        this->pos += d * this->right;
        this->needUpdate = true;
    }
    void Walk(float d)
    {
        this->pos += d * this->front;
        this->needUpdate = true;
    }
    void Zoom(float d)
    {
        this->distance-=d;
        this->Walk(d);
    }
    void Walking(float delta)
    {
        if (keys.front | keys.back | keys.left | keys.right | keys.up | keys.down)
        {
            Vector3 dir = (float)(keys.front - keys.back) * front + (float)(keys.right - keys.left) * right + (float)(keys.up - keys.down) * up;
            dir.Normalize();
            this->pos += delta * walkSpeed * dir;
            this->needUpdate = true;
        }
    }
    void Turn(float angleY, float angleX)
    {
        Matrix R = Matrix::CreateRotationY(turnSpeed * angleY);
        R = R * DirectX::XMMatrixRotationAxis(this->right, turnSpeed * angleX);
        this->right = DirectX::XMVector3TransformNormal(this->right, R);
        this->up = DirectX::XMVector3TransformNormal(this->up, R);
        this->front = DirectX::XMVector3TransformNormal(this->front, R);
        this->Reortho();
        this->needUpdate = true;
    }
    void Pitch(float angle)
    {
        Matrix R = DirectX::XMMatrixRotationAxis(this->right, turnSpeed * angle);
        this->up = DirectX::XMVector3TransformNormal(this->up, R);
        this->front = DirectX::XMVector3TransformNormal(this->front, R);
        this->Reortho();
        this->needUpdate = true;
    }
    void Turn(float angle)
    {
        Matrix R = Matrix::CreateRotationY(turnSpeed * angle);
        this->right = DirectX::XMVector3TransformNormal(this->right, R);
        this->up = DirectX::XMVector3TransformNormal(this->up, R);
        this->front = DirectX::XMVector3TransformNormal(this->front, R);
        this->Reortho();
        this->needUpdate = true;
    }
    void FocusOn(float angleY, float angleX)
    {
        Matrix R = Matrix::CreateRotationY(turnSpeed * angleY);
        R = R * DirectX::XMMatrixRotationAxis(this->right, turnSpeed * angleX);
        Vector3 target=this->GetTarget();
        this->right = DirectX::XMVector3TransformNormal(this->right, R);
        this->up = DirectX::XMVector3TransformNormal(this->up, R);
        this->front = DirectX::XMVector3TransformNormal(this->front, R);
        this->Reortho();
        this->pos=target-front*distance;
        this->needUpdate = true;
    }
    SkCamera(/* args */) {}
    ~SkCamera() {}
};
