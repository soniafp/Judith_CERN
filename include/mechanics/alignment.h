#ifndef ALIGNMENT_H
#define ALIGNMENT_H

namespace Mechanics {

class Alignment {
public:
  enum AlignAxis {
    OFFX,
    OFFY,
    OFFZ,
    ROTX,
    ROTY,
    ROTZ,
  };

private:
  bool m_disableOff;
  bool m_disableRot;

  void calculate();

protected:
  double m_alignment[6];
  double m_matrix[3][3];

public:
  Alignment();
  ~Alignment() {}

  void transform(double& x, double& y, double& z, bool inverse=false) const;
  void transform(double* values, bool inverse=false) const;

  void setAlignment(double* values);
  void setAlignment(AlignAxis axis, double value);
  void setOffX(double value);
  void setOffY(double value);
  void setOffZ(double value);
  void setRotX(double value);
  void setRotY(double value);
  void setRotZ(double value);

  inline void toggleOffset(bool value=true) { m_disableOff = !value; }
  inline void toggleRotation(bool value=true) { m_disableRot = !value; }

  inline double getAlignment(AlignAxis axis) const { return m_alignment[axis]; }
  inline double getOffX() const { return m_alignment[OFFX]; }
  inline double getOffY() const { return m_alignment[OFFY]; }
  inline double getOffZ() const { return m_alignment[OFFZ]; }
  inline double getRotX() const { return m_alignment[ROTX]; }
  inline double getRotY() const { return m_alignment[ROTY]; }
  inline double getRotZ() const { return m_alignment[ROTZ]; }
};

}

#endif  // ALIGNMENT_H

