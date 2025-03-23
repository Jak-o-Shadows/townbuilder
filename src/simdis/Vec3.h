/* -*- mode: c++ -*- */
/****************************************************************************
 *****                                                                  *****
 *****                   Classification: UNCLASSIFIED                   *****
 *****                    Classified By:                                *****
 *****                    Declassify On:                                *****
 *****                                                                  *****
 ****************************************************************************
 *
 *
 * Developed by: Naval Research Laboratory, Tactical Electronic Warfare Div.
 *               EW Modeling & Simulation, Code 5773
 *               4555 Overlook Ave.
 *               Washington, D.C. 20375-5339
 *
 * License for source code is in accompanying LICENSE.txt file. If you did
 * not receive a LICENSE.txt with this code, email simdis@us.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMCORE_CALC_VEC3_H
#define SIMCORE_CALC_VEC3_H

#include <Eigen/Dense>

#include <cassert>
#include <cmath>
#include <cstdlib>

namespace simCore {

// Header only class; doesn't need to be exported
/// (static sized) vector of three doubles
class Vec3 : public Eigen::Vector3d
{
public:
  using Eigen::Vector3d::Vector3d;

  /**
   * Value constructor
   * @param[in] v Vector elements
   * @pre valid input pointer, if not valid internal elements are set to zero
   */
  explicit Vec3(const double v[3])
  {
    if (v == nullptr)
    {
      zero();
      return;
    }
    set(v[0], v[1], v[2]);
  }

  /// Set vector elements to zero
  void zero() {
    x() = 0;
    y() = 0;
    z() = 0; 
  }

  /// Copy contents to a double[3] pointer
  void toD3(double dVec[3]) const
  {
    if (dVec == nullptr) return;
    dVec[0] = x();
    dVec[1] = y();
    dVec[2] = z();
  }

  /// Set first element component
  void setV0(double value) { x() = value; }
  /// Set second element component
  void setV1(double value) { y() = value; }
  /// Set third element component
  void setV2(double value) { z() = value; }

  /// Set all elements
  void set(double v0, double v1, double v2) { x() = v0; y() = v1; z() = v2; }
  /// Set all elements
  void set(const Vec3& value) { x() = value[0]; y() = value[1]; z() = value[2]; }

  /// Scales all elements
  void scale(double value) { x() *= value; y() *= value; z() *= value; }
  ///@}

  /**@name Mappings for {yaw,pitch,roll}, etc.
   * @{
   */

  double lat() const { return x(); }
  double lon() const { return y(); }
  double alt() const { return z(); }

  double range() const { return coeff(0); }
  double raeAz() const { return coeff(1); }
  double raeEl() const { return coeff(2); }

  double yaw() const { return coeff(0); }
  double pitch() const { return coeff(1); }
  double roll() const { return coeff(2); }

  double psi() const { return coeff(0); }
  double theta() const { return coeff(1); }
  double phi() const { return coeff(2); }

  void setX(double value) { setV0(value); }
  void setY(double value) { setV1(value); }
  void setZ(double value) { setV2(value); }

  void setLat(double value) { setV0(value); }
  void setLon(double value) { setV1(value); }
  void setAlt(double value) { setV2(value); }

  void setRange(double value) { setV0(value); }
  void setRaeAz(double value) { setV1(value); }
  void setRaeEl(double value) { setV2(value); }

  void setYaw(double value) { setV0(value); }
  void setPitch(double value) { setV1(value); }
  void setRoll(double value) { setV2(value); }

  void setPsi(double value) { setV0(value); }
  void setTheta(double value) { setV1(value); }
  void setPhi(double value) { setV2(value); }
  ///@}
};

} // namespace simCore

#endif /* SIMCORE_CALC_VEC3_H */
