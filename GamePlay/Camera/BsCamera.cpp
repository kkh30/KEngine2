//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsCamera.h"
#include <Math\BsRect2I.h>
namespace bs
{
	const float CameraBase::INFINITE_FAR_PLANE_ADJUST = 0.00001f;

	CameraBase::CameraBase()
		: mLayers(0xFFFFFFFFFFFFFFFF), mProjType(PT_PERSPECTIVE), mHorzFOV(Degree(90.0f)), mFarDist(500.0f)
		, mNearDist(0.05f), mAspect(1.33333333333333f), mOrthoHeight(5), mPriority(0), mCustomViewMatrix(false)
		, mCustomProjMatrix(false), mMSAA(1), mFrustumExtentsManuallySet(false), mProjMatrixRS(BsZero), mProjMatrix(BsZero)
		, mViewMatrix(BsZero), mProjMatrixRSInv(BsZero), mProjMatrixInv(BsZero), mViewMatrixInv(BsZero)
		, mRecalcFrustum(true), mRecalcFrustumPlanes(true), mRecalcView(true)
	{
		invalidateFrustum();
	}

	void CameraBase::setHorzFOV(const Radian& fov)
	{
		mHorzFOV = fov;
		invalidateFrustum();
		 
	}

	const Radian& CameraBase::getHorzFOV() const
	{
		return mHorzFOV;
	}

	void CameraBase::setFarClipDistance(float farPlane)
	{
		mFarDist = farPlane;
		invalidateFrustum();
		 
	}

	float CameraBase::getFarClipDistance() const
	{
		return mFarDist;
	}

	void CameraBase::setNearClipDistance(float nearPlane)
	{
		if (nearPlane <= 0)
		{
			assert("Near clip distance must be greater than zero.");
			return;
		}

		mNearDist = nearPlane;
		invalidateFrustum();
		 
	}

	float CameraBase::getNearClipDistance() const
	{
		return mNearDist;
	}

	const Matrix4& CameraBase::getProjectionMatrix() const
	{
		updateFrustum();

		return mProjMatrix;
	}

	const Matrix4& CameraBase::getProjectionMatrixInv() const
	{
		updateFrustum();

		return mProjMatrixInv;
	}

	const Matrix4& CameraBase::getProjectionMatrixRS() const
	{
		updateFrustum();

		return mProjMatrixRS;
	}

	const Matrix4& CameraBase::getProjectionMatrixRSInv() const
	{
		updateFrustum();

		return mProjMatrixRSInv;
	}

	const Matrix4& CameraBase::getViewMatrix() const
	{
		updateView();

		return mViewMatrix;
	}

	const Matrix4& CameraBase::getViewMatrixInv() const
	{
		updateView();

		return mViewMatrixInv;
	}

	const ConvexVolume& CameraBase::getFrustum() const
	{
		// Make any pending updates to the calculated frustum planes
		updateFrustumPlanes();

		return mFrustum;
	}

	ConvexVolume CameraBase::getWorldFrustum() const
	{
		const Vector<Plane>& frustumPlanes = getFrustum().getPlanes();

		const Transform& tfrm = mTransform;

		Matrix4 worldMatrix;
		worldMatrix.setTRS(tfrm.getPosition(), tfrm.getRotation(), Vector3::ONE);

		Vector<Plane> worldPlanes(frustumPlanes.size());
		UINT32 i = 0;
		for (auto& plane : frustumPlanes)
		{
			worldPlanes[i] = worldMatrix.multiplyAffine(plane);
			i++;
		}

		return ConvexVolume(worldPlanes);
	}

	void CameraBase::calcProjectionParameters(float& left, float& right, float& bottom, float& top) const
	{
		if (mCustomProjMatrix)
		{
			// Convert clipspace corners to camera space
			Matrix4 invProj = mProjMatrix.inverse();
			Vector3 topLeft(-0.5f, 0.5f, 0.0f);
			Vector3 bottomRight(0.5f, -0.5f, 0.0f);

			topLeft = invProj.multiply(topLeft);
			bottomRight = invProj.multiply(bottomRight);

			left = topLeft.x;
			top = topLeft.y;
			right = bottomRight.x;
			bottom = bottomRight.y;
		}
		else
		{
			if (mFrustumExtentsManuallySet)
			{
				left = mLeft;
				right = mRight;
				top = mTop;
				bottom = mBottom;
			}
			else if (mProjType == PT_PERSPECTIVE)
			{
				Radian thetaX(mHorzFOV * 0.5f);
				float tanThetaX = Math::tan(thetaX);
				float tanThetaY = tanThetaX / mAspect;

				float half_w = tanThetaX * mNearDist;
				float half_h = tanThetaY * mNearDist;

				left = -half_w;
				right = half_w;
				bottom = -half_h;
				top = half_h;

				mLeft = left;
				mRight = right;
				mTop = top;
				mBottom = bottom;
			}
			else
			{
				float half_w = getOrthoWindowWidth() * 0.5f;
				float half_h = getOrthoWindowHeight() * 0.5f;

				left = -half_w;
				right = half_w;
				bottom = -half_h;
				top = half_h;

				mLeft = left;
				mRight = right;
				mTop = top;
				mBottom = bottom;
			}
		}
	}

	void CameraBase::updateFrustum() const
	{
		if (isFrustumOutOfDate())
		{
			float left, right, bottom, top;

			calcProjectionParameters(left, right, bottom, top);

			if (!mCustomProjMatrix)
			{
				float inv_w = 1 / (right - left);
				float inv_h = 1 / (top - bottom);
				float inv_d = 1 / (mFarDist - mNearDist);

				if (mProjType == PT_PERSPECTIVE)
				{
					float A = 2 * mNearDist * inv_w;
					float B = 2 * mNearDist * inv_h;
					float C = (right + left) * inv_w;
					float D = (top + bottom) * inv_h;
					float q, qn;

					if (mFarDist == 0)
					{
						// Infinite far plane
						q = CameraBase::INFINITE_FAR_PLANE_ADJUST - 1;
						qn = mNearDist * (CameraBase::INFINITE_FAR_PLANE_ADJUST - 2);
					}
					else
					{
						q = -(mFarDist + mNearDist) * inv_d;
						qn = -2 * (mFarDist * mNearDist) * inv_d;
					}

					mProjMatrix = Matrix4::ZERO;
					mProjMatrix[0][0] = A;
					mProjMatrix[0][2] = C;
					mProjMatrix[1][1] = B;
					mProjMatrix[1][2] = D;
					mProjMatrix[2][2] = q;
					mProjMatrix[2][3] = qn;
					mProjMatrix[3][2] = -1;
				}
				else if (mProjType == PT_ORTHOGRAPHIC)
				{
					float A = 2 * inv_w;
					float B = 2 * inv_h;
					float C = -(right + left) * inv_w;
					float D = -(top + bottom) * inv_h;
					float q, qn;

					if (mFarDist == 0)
					{
						// Can not do infinite far plane here, avoid divided zero only
						q = -CameraBase::INFINITE_FAR_PLANE_ADJUST / mNearDist;
						qn = -CameraBase::INFINITE_FAR_PLANE_ADJUST - 1;
					}
					else
					{
						q = -2 * inv_d;
						qn = -(mFarDist + mNearDist)  * inv_d;
					}

					mProjMatrix = Matrix4::ZERO;
					mProjMatrix[0][0] = A;
					mProjMatrix[0][3] = C;
					mProjMatrix[1][1] = B;
					mProjMatrix[1][3] = D;
					mProjMatrix[2][2] = q;
					mProjMatrix[2][3] = qn;
					mProjMatrix[3][3] = 1;
				}
			}
			mProjMatrixInv = mProjMatrix.inverse();
			mProjMatrixRSInv = mProjMatrixRS.inverse();

			// Calculate bounding box (local)
			// Box is from 0, down -Z, max dimensions as determined from far plane
			// If infinite view frustum just pick a far value
			float farDist = (mFarDist == 0) ? 100000 : mFarDist;

			// Near plane bounds
			Vector3 min(left, bottom, -farDist);
			Vector3 max(right, top, 0);

			if (mCustomProjMatrix)
			{
				// Some custom projection matrices can have unusual inverted settings
				// So make sure the AABB is the right way around to start with
				Vector3 tmp = min;
				min.floor(max);
				max.ceil(tmp);
			}

			if (mProjType == PT_PERSPECTIVE)
			{
				// Merge with far plane bounds
				float radio = farDist / mNearDist;
				min.floor(Vector3(left * radio, bottom * radio, -farDist));
				max.ceil(Vector3(right * radio, top * radio, 0));
			}

			mBoundingBox.setExtents(min, max);

			mRecalcFrustum = false;
			mRecalcFrustumPlanes = true;
		}
	}

	bool CameraBase::isFrustumOutOfDate() const
	{
		return mRecalcFrustum;
	}

	void CameraBase::updateView() const
	{
		if (!mCustomViewMatrix && mRecalcView)
		{
			mViewMatrix.makeView(mTransform.getPosition(), mTransform.getRotation());
			mViewMatrixInv = mViewMatrix.inverseAffine();
			mRecalcView = false;
		}
	}

	void CameraBase::updateFrustumPlanes() const
	{
		updateFrustum();

		if (mRecalcFrustumPlanes)
		{
			mFrustum = ConvexVolume(mProjMatrix);
			mRecalcFrustumPlanes = false;
		}
	}

	float CameraBase::getAspectRatio() const
	{
		return mAspect;
	}

	void CameraBase::setAspectRatio(float r)
	{
		mAspect = r;
		invalidateFrustum();
		 
	}

	const AABox& CameraBase::getBoundingBox() const
	{
		updateFrustum();

		return mBoundingBox;
	}

	void CameraBase::setProjectionType(ProjectionType pt)
	{
		mProjType = pt;
		invalidateFrustum();
		 
	}

	ProjectionType CameraBase::getProjectionType() const
	{
		return mProjType;
	}

	void CameraBase::setCustomViewMatrix(bool enable, const Matrix4& viewMatrix)
	{
		mCustomViewMatrix = enable;
		if (enable)
		{
			mViewMatrix = viewMatrix;
			mViewMatrixInv = mViewMatrix.inverseAffine();
		}

		 
	}

	void CameraBase::setCustomProjectionMatrix(bool enable, const Matrix4& projMatrix)
	{
		mCustomProjMatrix = enable;

		if (enable)
			mProjMatrix = projMatrix;

		invalidateFrustum();
		 
	}

	void CameraBase::setOrthoWindow(float w, float h)
	{
		mOrthoHeight = h;
		mAspect = w / h;

		invalidateFrustum();
		 
	}

	void CameraBase::setOrthoWindowHeight(float h)
	{
		mOrthoHeight = h;

		invalidateFrustum();
		 
	}

	void CameraBase::setOrthoWindowWidth(float w)
	{
		mOrthoHeight = w / mAspect;

		invalidateFrustum();
		 
	}

	float CameraBase::getOrthoWindowHeight() const
	{
		return mOrthoHeight;
	}

	float CameraBase::getOrthoWindowWidth() const
	{
		return mOrthoHeight * mAspect;
	}

	void CameraBase::setFrustumExtents(float left, float right, float top, float bottom)
	{
		mFrustumExtentsManuallySet = true;
		mLeft = left;
		mRight = right;
		mTop = top;
		mBottom = bottom;

		invalidateFrustum();
		 
	}

	void CameraBase::resetFrustumExtents()
	{
		mFrustumExtentsManuallySet = false;

		invalidateFrustum();
		 
	}

	void CameraBase::getFrustumExtents(float& outleft, float& outright, float& outtop, float& outbottom) const
	{
		updateFrustum();

		outleft = mLeft;
		outright = mRight;
		outtop = mTop;
		outbottom = mBottom;
	}

	void CameraBase::setTransform(const Transform& transform)
	{
		setTransform(transform);
		
		mRecalcView = true;
	}

	void CameraBase::invalidateFrustum() const
	{
		mRecalcFrustum = true;
		mRecalcFrustumPlanes = true;
	}

	Vector2I CameraBase::worldToScreenPoint(const Vector3& worldPoint) const
	{
		Vector2 ndcPoint = worldToNdcPoint(worldPoint);
		return ndcToScreenPoint(ndcPoint);
	}

	Vector2 CameraBase::worldToNdcPoint(const Vector3& worldPoint) const
	{
		Vector3 viewPoint = worldToViewPoint(worldPoint);
		return viewToNdcPoint(viewPoint);
	}

	Vector3 CameraBase::worldToViewPoint(const Vector3& worldPoint) const
	{
		return getViewMatrix().multiplyAffine(worldPoint);
	}

	Vector3 CameraBase::screenToWorldPoint(const Vector2I& screenPoint, float depth) const
	{
		Vector2 ndcPoint = screenToNdcPoint(screenPoint);
		return ndcToWorldPoint(ndcPoint, depth);
	}

	Vector3 CameraBase::screenToWorldPointDeviceDepth(const Vector2I& screenPoint, float deviceDepth) const
	{
		Vector2 ndcPoint = screenToNdcPoint(screenPoint);
		Vector4 worldPoint(ndcPoint.x, ndcPoint.y, deviceDepth, 1.0f);
		worldPoint = getProjectionMatrixRS().inverse().multiply(worldPoint);

		Vector3 worldPoint3D;
		if (Math::abs(worldPoint.w) > 1e-7f)
		{
			float invW = 1.0f / worldPoint.w;

			worldPoint3D.x = worldPoint.x * invW;
			worldPoint3D.y = worldPoint.y * invW;
			worldPoint3D.z = worldPoint.z * invW;
		}

		return viewToWorldPoint(worldPoint3D);
	}

	Vector3 CameraBase::screenToViewPoint(const Vector2I& screenPoint, float depth) const
	{
		Vector2 ndcPoint = screenToNdcPoint(screenPoint);
		return ndcToViewPoint(ndcPoint, depth);
	}

	Vector2 CameraBase::screenToNdcPoint(const Vector2I& screenPoint) const
	{
		Rect2I viewport = getViewportRect();

		Vector2 ndcPoint;
		ndcPoint.x = (float)(((screenPoint.x - viewport.x) / (float)viewport.width) * 2.0f - 1.0f);

		//const RenderAPIInfo& info = RenderAPI::getAPIInfo();
		//if(info.isFlagSet(RenderAPIFeatureFlag::NDCYAxisDown))
		//	ndcPoint.y = (float)(((screenPoint.y - viewport.y) / (float)viewport.height) * 2.0f - 1.0f);
		//else
		ndcPoint.y = (float)((1.0f - ((screenPoint.y - viewport.y) / (float)viewport.height)) * 2.0f - 1.0f);

		return ndcPoint;
	}

	Vector3 CameraBase::viewToWorldPoint(const Vector3& viewPoint) const
	{
		return getViewMatrix().inverseAffine().multiplyAffine(viewPoint);
	}

	Vector2I CameraBase::viewToScreenPoint(const Vector3& viewPoint) const
	{
		Vector2 ndcPoint = viewToNdcPoint(viewPoint);
		return ndcToScreenPoint(ndcPoint);
	}

	Vector2 CameraBase::viewToNdcPoint(const Vector3& viewPoint) const
	{
		Vector3 projPoint = projectPoint(viewPoint);

		return Vector2(projPoint.x, projPoint.y);
	}

	Vector3 CameraBase::ndcToWorldPoint(const Vector2& ndcPoint, float depth) const
	{
		Vector3 viewPoint = ndcToViewPoint(ndcPoint, depth);
		return viewToWorldPoint(viewPoint);
	}

	Vector3 CameraBase::ndcToViewPoint(const Vector2& ndcPoint, float depth) const
	{
		return unprojectPoint(Vector3(ndcPoint.x, ndcPoint.y, depth));
	}

	Vector2I CameraBase::ndcToScreenPoint(const Vector2& ndcPoint) const
	{
		Rect2I viewport = getViewportRect();

		Vector2I screenPoint;
		screenPoint.x = Math::roundToInt(viewport.x + ((ndcPoint.x + 1.0f) * 0.5f) * viewport.width);
		screenPoint.y = Math::roundToInt(viewport.y + (1.0f - (ndcPoint.y + 1.0f) * 0.5f) * viewport.height);

		return screenPoint;
	}

	Ray CameraBase::screenPointToRay(const Vector2I& screenPoint) const
	{
		Vector2 ndcPoint = screenToNdcPoint(screenPoint);

		Vector3 near = unprojectPoint(Vector3(ndcPoint.x, ndcPoint.y, mNearDist));
		Vector3 far = unprojectPoint(Vector3(ndcPoint.x, ndcPoint.y, mNearDist + 1.0f));

		Ray ray(near, Vector3::normalize(far - near));
		ray.transformAffine(getViewMatrix().inverseAffine());

		return ray;
	}

	Vector3 CameraBase::projectPoint(const Vector3& point) const
	{
		Vector4 projPoint4(point.x, point.y, point.z, 1.0f);
		projPoint4 = getProjectionMatrixRS().multiply(projPoint4);

		if (Math::abs(projPoint4.w) > 1e-7f)
		{
			float invW = 1.0f / projPoint4.w;
			projPoint4.x *= invW;
			projPoint4.y *= invW;
			projPoint4.z *= invW;
		}
		else
		{
			projPoint4.x = 0.0f;
			projPoint4.y = 0.0f;
			projPoint4.z = 0.0f;
		}

		return Vector3(projPoint4.x, projPoint4.y, projPoint4.z);
	}

	Vector3 CameraBase::unprojectPoint(const Vector3& point) const
	{
		// Point.z is expected to be in view space, so we need to do some extra work to get the proper coordinates
		// (as opposed to if point.z was in device coordinates, in which case we could just inverse project)

		// Get world position for a point near the far plane (0.95f)
		Vector4 farAwayPoint(point.x, point.y, 0.95f, 1.0f);
		farAwayPoint = getProjectionMatrixRS().inverse().multiply(farAwayPoint);

		// Can't proceed if w is too small
		if (Math::abs(farAwayPoint.w) > 1e-7f)
		{
			// Perspective divide, to get the values that make sense in 3D space
			float invW = 1.0f / farAwayPoint.w;
			
			Vector3 farAwayPoint3D;
			farAwayPoint3D.x = farAwayPoint.x * invW;
			farAwayPoint3D.y = farAwayPoint.y * invW;
			farAwayPoint3D.z = farAwayPoint.z * invW;

			// Find the distance to the far point along the camera's viewing axis
			float distAlongZ = farAwayPoint3D.dot(-Vector3::UNIT_Z);

			// Do nothing if point is behind the camera
			if (distAlongZ >= 0.0f)
			{
				if (mProjType == PT_PERSPECTIVE)
				{
					// Direction from origin to our point
					Vector3 dir = farAwayPoint3D; // Camera is at (0, 0, 0) so it's the same vector

					// Our view space depth (point.z) is distance along the camera's viewing axis. Since our direction
					// vector is not parallel to the viewing axis, instead of normalizing it with its own length, we
					// "normalize" with the length projected along the camera's viewing axis.
					dir /= distAlongZ;

					// And now we just find the final position along the direction
					return dir * point.z;
				}
				else // Ortographic
				{
					// Depth difference between our arbitrary point and actual depth
					float depthDiff = distAlongZ - point.z;

					// Depth difference along viewing direction
					Vector3 depthDiffVec = depthDiff * -Vector3::UNIT_Z;

					// Return point that is depthDiff closer than our arbitrary point
					return farAwayPoint3D - depthDiffVec;
				}
			}
		}

		return Vector3(0.0f, 0.0f, 0.0f);
	}
}
