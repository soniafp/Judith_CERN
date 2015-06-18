#include <iostream>
#include <math.h>

#include "mechanics/alignment.h"

namespace Mechanics {

Alignment::Alignment() :
    // By default, the alignment applies offsets and rotations
    m_disableOff(false),
    m_disableRot(false) {
  // Initialize the alignment values to 0
  for (unsigned i = 0; i < 6; i++)
    m_alignment[i] = 0;
  // Initialize the rotation matrix to a diagonal one
  for (unsigned i = 0; i < 3; i++)
    for (unsigned j = 0; j < 3; j++)
      m_matrix[i][j] = (i==j) ? 1 : 0;
}

void Alignment::calculate() {
  // Shorter names make it clearer
  const double rx = m_alignment[ROTX];
  const double ry = m_alignment[ROTY];
  const double rz = m_alignment[ROTZ];

  // Rotation matrix Rz * Ry * Rx, note the order matters
  m_matrix[0][0] = cos(ry) * cos(rz);
  m_matrix[0][1] = -cos(rx) * sin(rz) + sin(rx) * sin(ry) * cos(rz);
  m_matrix[0][2] = sin(rx) * sin(rz) + cos(rx) * sin(ry) * cos(rz);
  m_matrix[1][0] = cos(ry) * sin(rz);
  m_matrix[1][1] = cos(rx) * cos(rz) + sin(rx) * sin(ry) * sin(rz);
  m_matrix[1][2] = -sin(rx) * cos(rz) + cos(rx) * sin(ry) * sin(rz);
  m_matrix[2][0] = -sin(ry);
  m_matrix[2][1] = sin(rx) * cos(ry);
  m_matrix[2][2] = cos(rx) * cos(ry);
}

void Alignment::transform(double& x, double& y, double& z, bool inverse) const {
  // The transform is implemented in the overloaded version which takes an
  // an array, so set the values in an array
  double values[3] = { x, y, z };
  transform(values, inverse);
  // Read the array back into the provided variables
  x = values[0];
  y = values[1];
  z = values[2];
}

void Alignment::transform(double* values, bool inverse) const {
  // No transformation requested
  if (m_disableRot && m_disableOff) return;

  // Buffer for applying the rotation matrix (needs to keep the original values
  // while updating the new values)
  double buffer[3] = { 0 };

  // Perform only translations
  if (m_disableRot) {
    for (unsigned i = 0; i < 3; i++)
      // Subtract or add the offset depending on inverse or not
      values[i] += (inverse ? -1 : +1) * m_alignment[OFFX+i];
  }

  // Perform only rotations
  else if (m_disableOff) {
    // Compute the rotation into the buffer
    for (unsigned i = 0; i < 3; i++)
      for (unsigned j = 0; j < 3; j++)
        // The inverse rotation is just the same operation with the transpose
        buffer[i] += values[j] * (inverse ? m_matrix[j][i] : m_matrix[i][j]);
    // Mover buffer back into original values
    for (unsigned i = 0; i < 3; i++)
      values[i] = buffer[i];
  }

  // Perform both rotation and translation
  else {
    // For the non-inverse transformation: translate then rotate
    if (!inverse) {
      // Compute the rotation into the buffer
      for (unsigned i = 0; i < 3; i++)
        for (unsigned j = 0; j < 3; j++)
          buffer[i] += values[j] * m_matrix[i][j];
      // Apply the translation and move back into values
      for (unsigned i = 0; i < 3; i++)
        values[i] = buffer[i] + m_alignment[OFFX+i];
    }

    // For the inverse transformation: un-rotate then un-translate
    else {
      // Buffer the untranslated points, and set values to 0
      for (unsigned i = 0; i < 3; i++) {
        buffer[i] = values[i] - m_alignment[OFFX+i];
        values[i] = 0;
      }
      // Update the values with the un-rotation of the buffer
      for (unsigned i = 0; i < 3; i++)
        for (unsigned j = 0; j < 3; j++)
          values[i] += buffer[j] * m_matrix[j][i];
    }
  }
}

// NOTE: all these methods need to call the `calculate` method to update the
// rotation matrix

void Alignment::setAlignment(const double* values) {
  for (unsigned i = 0; i < 6; i++)
    m_alignment[i] = values[i];
  calculate();
}

void Alignment::setAlignment(AlignAxis axis, double value) {
  m_alignment[axis] = value;
  calculate();
}

void Alignment::setOffX(double value) { m_alignment[OFFX] = value; calculate(); }
void Alignment::setOffY(double value) { m_alignment[OFFY] = value; calculate(); }
void Alignment::setOffZ(double value) { m_alignment[OFFZ] = value; calculate(); }
void Alignment::setRotX(double value) { m_alignment[ROTX] = value; calculate(); }
void Alignment::setRotY(double value) { m_alignment[ROTY] = value; calculate(); }
void Alignment::setRotZ(double value) { m_alignment[ROTZ] = value; calculate(); }

}

