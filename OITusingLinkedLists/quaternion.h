#ifndef PI
#define PI (3.1415926535897932384626433832795028841971693993751f)
#endif

#define N_PI PI

#ifndef TINY
#define TINY (0.0000001f)
#endif
#define N_TINY TINY

#define n_max(a,b)      (((a) > (b)) ? (a) : (b))
#define n_min(a,b)      (((a) < (b)) ? (a) : (b))
#define n_abs(a)        (((a)<0.0f) ? (-(a)) : (a))
#define n_sgn(a)        (((a)<0.0f) ? (-1) : (1))
#define n_deg2rad(d)    (((d)*PI)/180.0f)
#define n_rad2deg(r)    (((r)*180.0f)/PI)
#define n_sin(x)        (float(sin(x)))
#define n_cos(x)        (float(cos(x)))
#define n_tan(x)        (float(tan(x)))
#define n_atan(x)       (float(atan(x)))
#define n_atan2(x,y)    (float(atan2(x,y)))
#define n_exp(x)        (float(exp(x)))
#define n_floor(x)      (float(floor(x)))
#define n_ceil(x)       (float(ceil(x)))
#define n_pow(x,y)      (float(pow(x,y)))

inline float n_sqrt(float x)
{
	if (x < 0.0f) x = (float) 0.0f;
	return (float)sqrt(x);
}

inline float n_acos(float x)
{
	if (x >  1.0f) x = 1.0f;
	if (x < -1.0f) x = -1.0f;
	return (float)acos(x);
}

inline DirectX::XMFLOAT3 _cross(const DirectX::XMFLOAT3& v0, const DirectX::XMFLOAT3& v1){
	return DirectX::XMFLOAT3
	   (v0.y * v1.z - v0.z * v1.y,
		v0.z * v1.x - v0.x * v1.z,
		v0.x * v1.y - v0.y * v1.x);
}

inline float _dot(const DirectX::XMFLOAT3& v0, const DirectX::XMFLOAT3& v1){
	return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
}

class quaternion
{
public:
	static const quaternion identity;

public:
	float x, y, z, w;

	//-- constructors -----------------------------------------------
	quaternion()
		: x(0.0f), y(0.0f), z(0.0f), w(1.0f)
	{}
	quaternion(float _x, float _y, float _z, float _w)
		: x(_x), y(_y), z(_z), w(_w)
	{}
	quaternion(const quaternion& q)
		: x(q.x), y(q.y), z(q.z), w(q.w)
	{}

	//-- setting elements -------------------------------------------
	void set(float _x, float _y, float _z, float _w) {
		x = _x;
		y = _y;
		z = _z;
		w = _w;
	}
	void set(const quaternion& q) {
		x = q.x;
		y = q.y;
		z = q.z;
		w = q.w;
	}

	//-- misc operations --------------------------------------------
	void ident() {
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
		w = 1.0f;
	}

	void conjugate() {
		x = -x;
		y = -y;
		z = -z;
	}

	void scale(float s) {
		x *= s;
		y *= s;
		z *= s;
		w *= s;
	}

	float norm() {
		return x*x + y*y + z*z + w*w;
	}

	float magnitude() {
		float n = norm();
		if (n > 0.0f) return n_sqrt(n);
		else          return 0.0f;
	}

	void invert() {
		float n = norm();
		if (n > 0.0f) scale(1.0f / norm());
		conjugate();
	}

	void normalize() {
		float l = magnitude();
		if (l > 0.0f) scale(1.0f / l);
		else          set(0.0f, 0.0f, 0.0f, 1.0f);
	}

	//-- operators --------------------------------------------------
	bool operator==(const quaternion& q) {
		return ((x == q.x) && (y == q.y) && (z == q.z) && (w == q.w)) ? true : false;
	}

	bool operator!=(const quaternion& q) {
		return ((x != q.x) || (y != q.y) || (z != q.z) || (w != q.w)) ? true : false;
	}

	const quaternion& operator+=(const quaternion& q) {
		x += q.x;
		y += q.y;
		z += q.z;
		w += q.w;
		return *this;
	}

	const quaternion& operator-=(const quaternion& q) {
		x -= q.x;
		y -= q.y;
		z -= q.z;
		w -= q.w;
		return *this;
	}

	const quaternion& operator*=(const quaternion& q) {
		float qx = w*q.x + x*q.w + y*q.z - z*q.y;
		float qy = w*q.y + y*q.w + z*q.x - x*q.z;
		float qz = w*q.z + z*q.w + x*q.y - y*q.x;
		float qw = w*q.w - x*q.x - y*q.y - z*q.z;
		x = qx;
		y = qy;
		z = qz;
		w = qw;
		return *this;
	}

	DirectX::XMFLOAT4 toVector4(){
		return DirectX::XMFLOAT4(x, y, z, w);
	}

	/// rotate vector by quaternion
	DirectX::XMFLOAT3 rotate(const DirectX::XMFLOAT3& v) {
		quaternion q(v.x * w + v.z * y - v.y * z,
			v.y * w + v.x * z - v.z * x,
			v.z * w + v.y * x - v.x * y,
			v.x * x + v.y * y + v.z * z);

		return DirectX::XMFLOAT3(w * q.x + x * q.w + y * q.z - z * q.y,
			w * q.y + y * q.w + z * q.x - x * q.z,
			w * q.z + z * q.w + x * q.y - y * q.x);
	}

	DirectX::XMFLOAT3 z_direction() const
	{
		return DirectX::XMFLOAT3(2 * w * y + 2 * x * z,
			2 * y * z - 2 * x * w,
			w * w + z * z - x * x - y * y);
	}

	DirectX::XMFLOAT3 x_direction() const
	{
		return DirectX::XMFLOAT3(w * w + x * x - y * y - z * z,
			2 * w * z + 2 * y * x,
			2 * x * z - 2 * y * w);
	}

	DirectX::XMFLOAT3 y_direction() const
	{
		return DirectX::XMFLOAT3(2 * y * x - 2 * z * w,
			w * w + y * y - z * z - x * x,
			2 * z * y + 2 * x * w);
	}

	/**
	Create a rotation from one vector to an other. Works only with unit vectors.
	See http://www.martinb.com/maths/algebra/vectors/angleBetween/index.htm for
	more information.
	@param from source unit vector
	@param to destination unit vector
	*/
	quaternion& set_from_axes(const DirectX::XMFLOAT3& from, const DirectX::XMFLOAT3& to)
	{
		DirectX::XMFLOAT3 c(_cross(from, to));
		set(c.x, c.y, c.z, _dot(from, to));
		w += 1.0f;      // reducing angle to halfangle
		if (w <= TINY) // angle close to PI
		{
			if ((from.z * from.z) > (from.x * from.x))
				set(0, from.z, -from.y, w);
			//from*vector3(1,0,0)
			else
				set(from.y, -from.x, 0, w);
			//from*vector3(0,0,1)
		}
		normalize();

		return *this;
	}

	/**
	Create a rotation from one vector to an other. Works with non unit vectors.
	See http://www.martinb.com/maths/algebra/vectors/angleBetween/index.htm for
	more information.
	@param from source vector
	@param to destination vector
	*/
	quaternion& set_from_axes2(const DirectX::XMFLOAT3& from, const DirectX::XMFLOAT3& to)
	{
		DirectX::XMFLOAT3 c(_cross(from, to));
		set(c.x, c.y, c.z, _dot(from, to));
		normalize();    // if "from" or "to" not unit, normalize quat
		w += 1.0f;      // reducing angle to halfangle
		if (w <= TINY) // angle close to PI
		{
			if ((from.z * from.z) > (from.x * from.x))
				set(0, from.z, -from.y, w);
			//from*vector3(1,0,0)
			else
				set(from.y, -from.x, 0, w);
			//from*vector3(0,0,1)
		}
		normalize();

		return *this;
	}

	//-- convert from euler angles ----------------------------------
	void set_rotate_axis_angle(const DirectX::XMFLOAT3& v, float a) {
		float sin_a = n_sin(a * 0.5f);
		float cos_a = n_cos(a * 0.5f);
		x = v.x * sin_a;
		y = v.y * sin_a;
		z = v.z * sin_a;
		w = cos_a;
	}

	void set_rotate_x(float a) {
		float sin_a = n_sin(a * 0.5f);
		float cos_a = n_cos(a * 0.5f);
		x = sin_a;
		y = 0.0f;
		z = 0.0f;
		w = cos_a;
	}

	void set_rotate_y(float a) {
		float sin_a = n_sin(a * 0.5f);
		float cos_a = n_cos(a * 0.5f);
		x = 0.0f;
		y = sin_a;
		z = 0.0f;
		w = cos_a;
	}

	void set_rotate_z(float a) {
		float sin_a = n_sin(a * 0.5f);
		float cos_a = n_cos(a * 0.5f);
		x = 0.0f;
		y = 0.0f;
		z = sin_a;
		w = cos_a;
	}

	void set_rotate_xyz(float ax, float ay, float az) {
		quaternion qx, qy, qz;
		qx.set_rotate_x(ax);
		qy.set_rotate_y(ay);
		qz.set_rotate_z(az);
		*this = qx;
		*this *= qy;
		*this *= qz;
	}

	//--- fuzzy compare operators -----------------------------------
	bool isequal(const quaternion& v, float tol) const
	{
		if (fabs(v.x - x) > tol)      return false;
		else if (fabs(v.y - y) > tol) return false;
		else if (fabs(v.z - z) > tol) return false;
		else if (fabs(v.w - w) > tol) return false;
		return true;
	}

	//-- rotation interpolation, set this matrix to the -------------
	//-- interpolated result of q0->q1 with l as interpolator -------
	void slerp(const quaternion& q0, const quaternion& q1, float l)
	{
		float fScale1;
		float fScale2;
		quaternion A = q0;
		quaternion B = q1;

		// compute dot product, aka cos(theta):
		float fCosTheta = A.x*B.x + A.y*B.y + A.z*B.z + A.w*B.w;

		if (fCosTheta < 0.0f)
		{
			// flip start quaternion
			A.x = -A.x; A.y = -A.y; A.z = -A.z; A.w = -A.w;
			fCosTheta = -fCosTheta;
		}

		if ((fCosTheta + 1.0f) > 0.05f)
		{
			// If the quaternions are close, use linear interploation
			if ((1.0f - fCosTheta) < 0.05f)
			{
				fScale1 = 1.0f - l;
				fScale2 = l;
			}
			else
			{
				// Otherwise, do spherical interpolation
				float fTheta = n_acos(fCosTheta);
				float fSinTheta = n_sin(fTheta);
				fScale1 = n_sin(fTheta * (1.0f - l)) / fSinTheta;
				fScale2 = n_sin(fTheta * l) / fSinTheta;
			}
		}
		else
		{
			B.x = -A.y;
			B.y = A.x;
			B.z = -A.w;
			B.w = A.z;
			fScale1 = n_sin(PI * (0.5f - l));
			fScale2 = n_sin(PI * l);
		}

		x = fScale1 * A.x + fScale2 * B.x;
		y = fScale1 * A.y + fScale2 * B.y;
		z = fScale1 * A.z + fScale2 * B.z;
		w = fScale1 * A.w + fScale2 * B.w;
	}

	void lerp(const quaternion& q0, const quaternion& q1, float l)
	{
		slerp(q0, q1, l);
	}
};
