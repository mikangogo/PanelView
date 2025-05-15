#pragma once

#include "PvPf_Windows.hpp"
#include "PvWindowBase.hpp"

#include <array>
#include <complex>
#include <numbers>

#include <d3dx9math.h>

typedef void* PvGrTextureHandle;
typedef void* PvGrContextHandle;

inline double PvGrToRadian(double degree)
{
    return degree * (std::numbers::pi / 180.0);
}

struct PvGrVector2D
{
    double X = 0.0;
    double Y = 0.0;

    void ToArray(float* target) const
    {
        target[0] = static_cast<float>(X);
        target[1] = static_cast<float>(Y);
    }

    PvGrVector2D& operator+=(const PvGrVector2D& v)
    {
        X += v.X;
        Y += v.Y;

        return *this;
    }

    PvGrVector2D& operator-=(const PvGrVector2D& v)
    {
        X -= v.X;
        Y -= v.Y;

        return *this;
    }

    PvGrVector2D& operator*=(const double scalar)
    {
        X *= scalar;
        Y *= scalar;

        return *this;
    }

    PvGrVector2D& operator/=(const double scalar)
    {
        const auto inv = 1.0 / scalar;
        X *= inv;
        Y *= inv;

        return *this;
    }

    PvGrVector2D operator+() const { return *this; }

    PvGrVector2D operator-() const { return {.X = X, .Y = Y}; }

    PvGrVector2D operator+(const PvGrVector2D& v) const { return {.X = X + v.X, .Y = Y + v.Y}; }

    PvGrVector2D operator-(const PvGrVector2D& v) const { return {.X = X - v.X, .Y = Y - v.Y}; }

    PvGrVector2D operator*(const double scalar) const { return {.X = X * scalar, .Y = Y * scalar}; }

    PvGrVector2D operator/(const double scalar) const
    {
        const auto inv = 1.0 / scalar;
        return {.X = X * inv, .Y = Y * inv};
    }

    bool operator==(const PvGrVector2D& v) const { return (X == v.X) && (Y == v.Y); }

    bool operator!=(const PvGrVector2D& v) const { return (X != v.X) || (Y != v.Y); }

    double Length() const { return std::sqrt(X * X + Y * Y); }

    PvGrVector2D Normalize() const
    {
        const auto length = Length();

        if (length > 0.0)
        {
            return *this / length;
        }

        return {};
    }
};

struct PvGrVector3D
{
    double X = 0.0;
    double Y = 0.0;
    double Z = 0.0;

    void ToArray(float* target) const
    {
        target[0] = static_cast<float>(X);
        target[1] = static_cast<float>(Y);
        target[2] = static_cast<float>(Z);
    }

    PvGrVector3D& operator+=(const PvGrVector3D& v)
    {
        X += v.X;
        Y += v.Y;
        Z += v.Z;

        return *this;
    }

    PvGrVector3D& operator-=(const PvGrVector3D& v)
    {
        X -= v.X;
        Y -= v.Y;
        Z -= v.Z;

        return *this;
    }

    PvGrVector3D& operator*=(const double scalar)
    {
        X *= scalar;
        Y *= scalar;
        Z *= scalar;

        return *this;
    }

    PvGrVector3D& operator/=(const double scalar)
    {
        const auto inv = 1.0 / scalar;
        X *= inv;
        Y *= inv;
        Z *= inv;

        return *this;
    }

    PvGrVector3D operator+() const
    {
        return *this;
    }

    PvGrVector3D operator-() const
    {
        return {.X = X, .Y = Y, .Z = Z};
    }

    PvGrVector3D operator+(const PvGrVector3D& v) const
    {
        return {.X = X + v.X, .Y = Y + v.Y, .Z = Z + v.Z};
    }

    PvGrVector3D operator-(const PvGrVector3D& v) const
    {
        return {.X = X - v.X, .Y = Y - v.Y, .Z = Z - v.Z};
    }

    PvGrVector3D operator*(const double scalar) const
    {
        return {.X = X * scalar, .Y = Y * scalar, .Z = Z * scalar};
    }

    PvGrVector3D operator/(const double scalar) const
    {
        const auto inv = 1.0 / scalar;
        return {.X = X * inv, .Y = Y * inv, .Z = Z * inv};
    }

    bool operator==(const PvGrVector3D& v) const
    {
        return (X == v.X) && (Y == v.Y) && (Z == v.Z);
    }

    bool operator!=(const PvGrVector3D& v) const
    {
        return (X != v.X) || (Y != v.Y) || (Z != v.Z);
    }

    double Length() const
    {
        return std::sqrt(X * X + Y * Y + Z * Z);
    }

    PvGrVector3D Normalize() const
    {
        const auto length = Length();

        if (length > 0.0)
        {
            return *this / length;
        }

        return {};
    }

    PvGrVector3D OuterProduct(const PvGrVector3D& v) const
    {
        return {.X = (Y * v.Z) - (Z * v.Y), .Y = (Z * v.X) - (X * v.Z), .Z = (X * v.Y) - (Y * v.X)};
    }

    double InnerProduct(const PvGrVector3D& v) const
    {
        return X * v.X + Y * v.Y + Z * v.Z;
    }
};

struct PvGrVector4D
{
    double X = 0.0;
    double Y = 0.0;
    double Z = 0.0;
    double W = 0.0;

    void ToFloatArray(float* target) const
    {
        target[0] = static_cast<float>(X);
        target[1] = static_cast<float>(Y);
        target[2] = static_cast<float>(Z);
        target[3] = static_cast<float>(W);
    }
};

struct PvGrSize2D
{
    unsigned int Width = 0;
    unsigned int Height = 0;
};

struct PvGrRectangle2D
{
    double X = 0.0;
    double Y = 0.0;
    double Width = 0.0;
    double Height = 0.0;

    double Left() const
    {
        return X;
    }

    double Right() const
    {
        return X + Width;
    }

    double Top() const
    {
        return Y;
    }

    double Bottom() const
    {
        return Y + Height;
    }
};

struct PvGrColor
{
    PvGrColor() = default;

    PvGrColor(const unsigned char a, const unsigned char r, const unsigned char g, const unsigned char b) { *this = FromArgb(a, r, g, b); }

    double R = 0.0;
    double G = 0.0;
    double B = 0.0;
    double A = 0.0;

    static PvGrColor FromArgb(const unsigned char a, const unsigned char r, const unsigned char g,
                              const unsigned char b)
    {
        PvGrColor color;

        color.A = Normalize(a);
        color.R = Normalize(r);
        color.G = Normalize(g);
        color.B = Normalize(b);

        return color;
    }

    static PvGrColor FromArgbFp64(const double a, const double r, const double g, const double b)
    {
        PvGrColor color;

        color.A = (a);
        color.R = (r);
        color.G = (g);
        color.B = (b);

        return color;
    }

    static PvGrColor FromRgb(const unsigned char r, const unsigned char g, const unsigned char b)
    {
        PvGrColor color;

        color.R = Normalize(r);
        color.G = Normalize(g);
        color.B = Normalize(b);

        return color;
    }

    void ToRgbaFloatArray(float* target) const
    {
        target[0] = static_cast<float>(R);
        target[1] = static_cast<float>(G);
        target[2] = static_cast<float>(B);
        target[3] = static_cast<float>(A);
    }

    void ToArgbFloatArray(float* target) const
    {
        target[0] = static_cast<float>(A);
        target[1] = static_cast<float>(R);
        target[2] = static_cast<float>(G);
        target[3] = static_cast<float>(B);
    }

    unsigned int ToArgb() const
    {
        return (static_cast<int>(A * 255.0) << 24) | (static_cast<int>(R * 255.0) << 16) |
            (static_cast<int>(G * 255.0) << 8) | (static_cast<int>(B * 255.0) << 0);
    }

private:
    static double Normalize(const unsigned char byteColor)
    {
        return static_cast<double>(byteColor) / 255.0;
    }
};

struct PvGrTransform
{
    PvGrVector4D X;
    PvGrVector4D Y;
    PvGrVector4D Z;
    PvGrVector4D W;

    static PvGrTransform Identity()
    {
        return
        {
            .X = {.X = 1.0, .Y = 0.0, .Z = 0.0, .W = 0.0},
            .Y = {.X = 0.0, .Y = 1.0, .Z = 0.0, .W = 0.0},
            .Z = {.X = 0.0, .Y = 0.0, .Z = 1.0, .W = 0.0},
            .W = {.X = 0.0, .Y = 0.0, .Z = 0.0, .W = 1.0}
        };
    }

    static PvGrTransform TranslationTransform(const PvGrVector3D& v)
    {
        auto mat = Identity();

        mat.W.X = v.X;
        mat.W.Y = v.Y;
        mat.W.Z = v.Z;

        return mat;
    }

    static PvGrTransform ScalingTransform(const PvGrVector3D& v)
    {
        auto mat = Identity();

        mat.X.X = v.X;
        mat.Y.Y = v.Y;
        mat.Z.Z = v.Z;

        return mat;
    }

    static PvGrTransform RotationXTransform(const double angle)
    {
        auto mat = Identity();

        auto c = cos(angle);
        auto s = sin(angle);
        
        mat.Y.Y = c;
        mat.Y.Z = s;
        mat.Z.Y = -s;
        mat.Z.Z = c;

        return mat;
    }

    static PvGrTransform RotationYTransform(const double angle)
    {
        auto mat = Identity();

        auto c = cos(angle);
        auto s = sin(angle);

        mat.X.X = c;
        mat.X.Z = s;
        mat.Z.X = -s;
        mat.Z.Z = c;

        return mat;
    }

    static PvGrTransform RotationZTransform(const double angle)
    {
        auto mat = Identity();

        auto c = cos(angle);
        auto s = sin(angle);

        mat.X.X = c;
        mat.X.Y = s;
        mat.Y.X = -s;
        mat.Y.Y = c;

        return mat;
    }

    static void Multiply(PvGrTransform& mOut, const PvGrTransform& m1, const PvGrTransform& m2)
    {
        mOut.X.X = m1.X.X * m2.X.X + m1.X.Y * m2.Y.X + m1.X.Z * m2.Z.X + m1.X.W * m2.W.X;
        mOut.X.Y = m1.X.X * m2.X.Y + m1.X.Y * m2.Y.Y + m1.X.Z * m2.Z.Y + m1.X.W * m2.W.Y;
        mOut.X.Z = m1.X.X * m2.X.Z + m1.X.Y * m2.Y.Z + m1.X.Z * m2.Z.Z + m1.X.W * m2.W.Z;
        mOut.X.W = m1.X.X * m2.X.W + m1.X.Y * m2.Y.W + m1.X.Z * m2.Z.W + m1.X.W * m2.W.W;

        mOut.Y.X = m1.Y.X * m2.X.X + m1.Y.Y * m2.Y.X + m1.Y.Z * m2.Z.X + m1.Y.W * m2.W.X;
        mOut.Y.Y = m1.Y.X * m2.X.Y + m1.Y.Y * m2.Y.Y + m1.Y.Z * m2.Z.Y + m1.Y.W * m2.W.Y;
        mOut.Y.Z = m1.Y.X * m2.X.Z + m1.Y.Y * m2.Y.Z + m1.Y.Z * m2.Z.Z + m1.Y.W * m2.W.Z;
        mOut.Y.W = m1.Y.X * m2.X.W + m1.Y.Y * m2.Y.W + m1.Y.Z * m2.Z.W + m1.Y.W * m2.W.W;

        mOut.Z.X = m1.Z.X * m2.X.X + m1.Z.Y * m2.Y.X + m1.Z.Z * m2.Z.X + m1.Z.W * m2.W.X;
        mOut.Z.Y = m1.Z.X * m2.X.Y + m1.Z.Y * m2.Y.Y + m1.Z.Z * m2.Z.Y + m1.Z.W * m2.W.Y;
        mOut.Z.Z = m1.Z.X * m2.X.Z + m1.Z.Y * m2.Y.Z + m1.Z.Z * m2.Z.Z + m1.Z.W * m2.W.Z;
        mOut.Z.W = m1.Z.X * m2.X.W + m1.Z.Y * m2.Y.W + m1.Z.Z * m2.Z.W + m1.Z.W * m2.W.W;

        mOut.W.X = m1.W.X * m2.X.X + m1.W.Y * m2.Y.X + m1.W.Z * m2.Z.X + m1.W.W * m2.W.X;
        mOut.W.Y = m1.W.X * m2.X.Y + m1.W.Y * m2.Y.Y + m1.W.Z * m2.Z.Y + m1.W.W * m2.W.Y;
        mOut.W.Z = m1.W.X * m2.X.Z + m1.W.Y * m2.Y.Z + m1.W.Z * m2.Z.Z + m1.W.W * m2.W.Z;
        mOut.W.W = m1.W.X * m2.X.W + m1.W.Y * m2.Y.W + m1.W.Z * m2.Z.W + m1.W.W * m2.W.W;
    }

    PvGrTransform operator*(const PvGrTransform& m) const
    {
        PvGrTransform mat;

        Multiply(mat, *this, m);

        return mat;
    }

    PvGrTransform& operator*=(const PvGrTransform& m)
    {
        PvGrTransform mat;

        Multiply(mat, *this, m);

        *this = mat;

        return *this;
    }

    static bool Inverse(PvGrTransform& mOut, const PvGrTransform& m)
    {
        auto& m00 = m.X.X;
        auto& m01 = m.Y.X;
        auto& m02 = m.Z.X;
        auto& m03 = m.W.X;
        auto& m10 = m.X.Y;
        auto& m11 = m.Y.Y;
        auto& m12 = m.Z.Y;
        auto& m13 = m.W.Y;
        auto& m20 = m.X.Z;
        auto& m21 = m.Y.Z;
        auto& m22 = m.Z.Z;
        auto& m23 = m.W.Z;
        auto& m30 = m.X.W;
        auto& m31 = m.Y.W;
        auto& m32 = m.Z.W;
        auto& m33 = m.W.W;

        auto det3x3 =
            [](const double& a1, const double& a2, const double& a3, const double& b1, const double& b2, const double& b3, const double& c1, const double& c2, const double& c3)
        { return a1 * (b2 * c3 - b3 * c2) - a2 * (b1 * c3 - b3 * c1) + a3 * (b1 * c2 - b2 * c1); };

        auto c00 = det3x3(m11, m12, m13, m21, m22, m23, m31, m32, m33);
        auto c01 = -det3x3(m10, m12, m13, m20, m22, m23, m30, m32, m33);
        auto c02 = det3x3(m10, m11, m13, m20, m21, m23, m30, m31, m33);
        auto c03 = -det3x3(m10, m11, m12, m20, m21, m22, m30, m31, m32);

        auto c10 = -det3x3(m01, m02, m03, m21, m22, m23, m31, m32, m33);
        auto c11 = det3x3(m00, m02, m03, m20, m22, m23, m30, m32, m33);
        auto c12 = -det3x3(m00, m01, m03, m20, m21, m23, m30, m31, m33);
        auto c13 = det3x3(m00, m01, m02, m20, m21, m22, m30, m31, m32);

        auto c20 = det3x3(m01, m02, m03, m11, m12, m13, m31, m32, m33);
        auto c21 = -det3x3(m00, m02, m03, m10, m12, m13, m30, m32, m33);
        auto c22 = det3x3(m00, m01, m03, m10, m11, m13, m30, m31, m33);
        auto c23 = -det3x3(m00, m01, m02, m10, m11, m12, m30, m31, m32);

        auto c30 = -det3x3(m01, m02, m03, m11, m12, m13, m21, m22, m23);
        auto c31 = det3x3(m00, m02, m03, m10, m12, m13, m20, m22, m23);
        auto c32 = -det3x3(m00, m01, m03, m10, m11, m13, m20, m21, m23);
        auto c33 = det3x3(m00, m01, m02, m10, m11, m12, m20, m21, m22);

        auto det = m00 * c00 + m01 * c01 + m02 * c02 + m03 * c03;
        if (std::abs(det) < 1.0e-15)
        {
            return false;
        }

        auto invDet = 1.0 / det;
        PvGrTransform inv;

        mOut.X.X = c00 * invDet;
        mOut.Y.X = c10 * invDet;
        mOut.Z.X = c20 * invDet;
        mOut.W.X = c30 * invDet;

        mOut.X.Y = c01 * invDet;
        mOut.Y.Y = c11 * invDet;
        mOut.Z.Y = c21 * invDet;
        mOut.W.Y = c31 * invDet;

        mOut.X.Z = c02 * invDet;
        mOut.Y.Z = c12 * invDet;
        mOut.Z.Z = c22 * invDet;
        mOut.W.Z = c32 * invDet;

        mOut.X.W = c03 * invDet;
        mOut.Y.W = c13 * invDet;
        mOut.Z.W = c23 * invDet;
        mOut.W.W = c33 * invDet;

        return true;
    }

    void ToFloatArray(float* target) const { X.ToFloatArray(&target[0]); Y.ToFloatArray(&target[4]); Z.ToFloatArray(&target[8]); W.ToFloatArray(&target[12]); }
};

struct PvGrPerspectiveOffCenterProjectionTransform : PvGrTransform
{
    double Left;
    double Right;
    double Bottom;
    double Top;
    double NearZ;
    double FarZ;

    void ApplyLeftHand()
    {
        // See also: https://github.com/MicrosoftDocs/win32/blob/docs/desktop-src/direct3d9/d3dxmatrixperspectiveoffcenterlh.md

        X.X = 2.0 * NearZ / (Right - Left);
        X.Y = 0;
        X.Z = 0;
        X.W = 0;

        Y.X = 0;
        Y.Y = 2.0 * NearZ / (Top - Bottom);
        Y.Z = 0;
        Y.W = 0;

        Z.X = (Left + Right) / (Left - Right);
        Z.Y = (Top + Bottom) / (Bottom - Top);
        Z.Z = FarZ / (FarZ - NearZ);
        Z.W = 1.0;

        W.X = 0;
        W.Y = 0;
        W.Z = NearZ * FarZ / (NearZ - FarZ);
        W.W = 0;
    }
};

struct PvGrPerspectiveProjectionTransform : PvGrTransform
{
    double FoV;
    double AspectRatio;
    double NearZ;
    double FarZ;

    void ApplyLeftHand()
    {
        // See also: https://github.com/MicrosoftDocs/win32/blob/docs/desktop-src/direct3d9/d3dxmatrixperspectivefovlh.md

        const double yScale = 1.0f / std::tan(FoV * 0.5);
        const double xScale = yScale / AspectRatio;

        X.X = xScale;
        X.Y = 0;
        X.Z = 0;
        X.W = 0;

        Y.X = 0;
        Y.Y = yScale;
        Y.Z = 0;
        Y.W = 0;

        Z.X = 0;
        Z.Y = 0;
        Z.Z = FarZ / (FarZ - NearZ);
        Z.W = 1.0f;

        W.X = 0;
        W.Y = 0;
        W.Z = -(NearZ * FarZ) / (FarZ - NearZ);
        W.W = 0;
    }
};

struct PvGrLookAtViewTransform : PvGrTransform
{
    PvGrVector3D Eye;
    PvGrVector3D At;
    PvGrVector3D Up;

    void Apply()
    {
        auto zaxis = (At - Eye).Normalize();
        auto xaxis = (Up.OuterProduct(zaxis)).Normalize();
        auto yaxis = zaxis.OuterProduct(xaxis);

        X.X = xaxis.X;
        X.Y = yaxis.X;
        X.Z = zaxis.X;
        X.W = 0.0;
        Y.X = xaxis.Y;
        Y.Y = yaxis.Y;
        Y.Z = zaxis.Y;
        Y.W = 0.0;
        Z.X = xaxis.Z;
        Z.Y = yaxis.Z;
        Z.Z = zaxis.Z;
        Z.W = 0.0;
        W.X = xaxis.InnerProduct(Eye) * -1.0;
        W.Y = yaxis.InnerProduct(Eye) * -1.0;
        W.Z = zaxis.InnerProduct(Eye) * -1.0;
        W.W = 1.0;
    }
};

struct PvGrViewPort
{
    double X = 0.0;
    double Y = 0.0;
    double Width = 1.0;
    double Height = 1.0;
    double MinimumZ = 0.0;
    double MaximumZ = 1.0;
};

enum PvGrPositionDataType
{
    PvGrPositionDataType_Invalid = -1,
    PvGrPositionDataType_3D_Color,
    PvGrPositionDataType_3D_Uv_Color,
    PvGrPositionDataType_MaxLength,
};

struct PvGrPositionData_3D
{
    PvGrPositionData_3D(const PvGrVector3D& position, const PvGrVector2D& uv, const PvGrColor& color)
    {
        position.ToArray(Position.data());
        uv.ToArray(Uv.data());
        Color = color.ToArgb();
    }

    std::array<float, 3> Position = {};
    std::array<float, 2> Uv = {};
    unsigned int Color = PvGrColor(255, 255, 255, 255).ToArgb();
};

extern bool pvGrStartContext(PvGrContextHandle context, const PvGrColor& clearColor);
extern void pvGrEndContext(PvGrContextHandle context);
extern bool pvGrResetContext(PvGrContextHandle context);
extern bool pvGrResetContext(PvGrContextHandle context, const PvWindowBase& window);

extern PvGrContextHandle pvGrCreateContext(const PvWindowBase& window, unsigned int targetAdaptorId = 0);
extern void pvGrDeleteContext(PvGrContextHandle context);

extern void pvGrInitialize();
extern void pvGrShutdown();

extern PvGrTextureHandle pvGrGenerateTextureFromFile(PvGrContextHandle context, const PvPfFileInfo& fileInfo, const PvGrColor& colorKey = PvGrColor(0, 0, 0, 0));
extern void pvGrDeleteTexture(PvGrTextureHandle texture);
extern void pvGrSetTexture(PvGrContextHandle context, PvGrTextureHandle texture);
extern void pvGrSetBlendingTexture(PvGrContextHandle context, PvGrTextureHandle texture1, PvGrTextureHandle texture2, double weight);
extern void pvGrGetTextureSize(PvGrTextureHandle texture, PvGrSize2D& size);

extern void pvGrSetProjectionTransform(PvGrContextHandle context, const PvGrTransform& transform);
extern void pvGrSetViewTransform(PvGrContextHandle context, const PvGrTransform& transform);
extern void pvGrSetModelTransform(PvGrContextHandle context, const PvGrTransform& transform);
extern void pvGrApplyModelTransformStack(PvGrContextHandle context);
extern void pvGrClearModelTransformStack(PvGrContextHandle context);
extern void pvGrPushModelTransformStack(PvGrContextHandle context, const PvGrTransform& transform);
extern void pvGrPopModelTransformStack(PvGrContextHandle context);

extern void pvGrSetViewport(PvGrContextHandle context, const PvGrViewPort& viewport);
extern void pvGrGetViewport(PvGrContextHandle context, PvGrViewPort& viewport);

extern void pvGrDrawTriangles(PvGrContextHandle context, const std::span<const PvGrPositionData_3D>& vertices);
extern void pvGrDrawRectangle(PvGrContextHandle context, double width, double height, const PvGrColor& color);
