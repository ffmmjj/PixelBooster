/***************************************************************************\
*  Pixel::Booster, a simple pixel art image editor.                         *
*  Copyright (C) 2015  Ricardo Bustamante de Queiroz (ricardo@busta.com.br) *
*  Visit the Official Homepage: pixel.busta.com.br                          *
*                                                                           *
*  This program is free software: you can redistribute it and/or modify     *
*  it under the terms of the GNU General Public License as published by     *
*  the Free Software Foundation, either version 3 of the License, or        *
*  (at your option) any later version.                                      *
*                                                                           *
*  This program is distributed in the hope that it will be useful,          *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
*  GNU General Public License for more details.                             *
*                                                                           *
*  You should have received a copy of the GNU General Public License        *
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
\***************************************************************************/

#include "tool_algorithm.h"

#include "utils/debug.h"
#include "widgets/image_edit_widget.h"

#include <QPainter>
#include <cstdlib>

void ToolAlgorithm::FloodFill(QImage *image, const QPoint &seed, const QColor &color) {
  QRgb new_color = color.rgba();
  QRgb old_color = image->pixel(seed);
  if (new_color == old_color) {
    return;
  }
  QList<QPoint> to_do_list = {seed};

  QVector<QPoint> expansion = {
      QPoint(1, 0),
      QPoint(0, 1),
      QPoint(-1, 0),
      QPoint(0, -1)};

  image->setPixel(seed, new_color);

  while (!to_do_list.isEmpty()) {
    //DEBUG_MSG(to_do_list.length());
    QPoint target = to_do_list.takeFirst();

    for (QPoint e : expansion) {
      QPoint new_target = target + e;
      //DEBUG_MSG(new_target);
      if (image->rect().contains(new_target) && image->pixel(new_target) == old_color) {
        to_do_list.push_back(new_target);
        image->setPixel(new_target, new_color);
      }
    }
  }
}

void ToolAlgorithm::BresenhamLine(QImage *image, const QPoint &p1, const QPoint &p2, const QRgb &color) {
  // Algorithm taken from http://www.roguebasin.com/index.php?title=Bresenham%27s_Line_Algorithm

  int x1 = p1.x();
  int y1 = p1.y();
  const int x2 = p2.x();
  const int y2 = p2.y();

  int delta_x(x2 - x1);
  signed char const ix((delta_x > 0) - (delta_x < 0));
  delta_x = std::abs(delta_x) << 1;

  int delta_y(y2 - y1);
  signed char const iy((delta_y > 0) - (delta_y < 0));
  delta_y = std::abs(delta_y) << 1;

  SetPixel(image, x1, y1, color);

  if (delta_x >= delta_y) {
    int error(delta_y - (delta_x >> 1));
    while (x1 != x2) {
      if ((error >= 0) && (error || (ix > 0))) {
        error -= delta_x;
        y1 += iy;
      }
      error += delta_y;
      x1 += ix;

      SetPixel(image, x1, y1, color);
    }
  } else {
    int error(delta_x - (delta_y >> 1));
    while (y1 != y2) {
      if ((error >= 0) && (error || (iy > 0))) {
        error -= delta_y;
        x1 += ix;
      }
      error += delta_x;
      y1 += iy;

      SetPixel(image, x1, y1, color);
    }
  }
}

void ToolAlgorithm::BresenhamEllipse(QImage *image, const QRect &rect, bool fill, const QRgb &color) {
  // Algorithm from https://web.archive.org/web/20120225095359/http://homepage.smc.edu/kennedy_john/belipse.pdf
  QPoint c = rect.center();
  // Checks if the rect size is even on both directions
  QPoint e = QPoint(1 - rect.width() % 2, 1 - rect.height() % 2);
  int r_x = rect.width() / 2;
  int r_y = rect.height() / 2;

  if (rect.width() <= 0 || rect.height() <= 0) {
    // Avoid drawing ellipses with area 0
    return;
  }

  int x = r_x;
  int y = 0;
  int x_change = r_y * r_y * (1 - 2 * r_x);
  int y_change = r_x * r_x;
  int ellipse_error = 0;
  int two_a_square = 2 * r_x * r_x;
  int two_b_square = 2 * r_y * r_y;
  int stopping_x = two_b_square * r_x;
  int stopping_y = 0;

  QPoint last_h;
  QPoint last_v;

  // Drawing horizontal portion of the ellipse
  while (stopping_x >= stopping_y) {
    last_h = QPoint(x, y);
    if (fill) {
      for (int i = 0; i < last_h.x(); i++) {
        Plot4EllipsePoints(image, c, QPoint(i, last_h.y()), e, color);
      }
    } else {
      Plot4EllipsePoints(image, c, last_h, e, color);
    }
    y++;
    stopping_y += two_a_square;
    ellipse_error += y_change;
    y_change += two_a_square;
    if ((2 * ellipse_error + x_change) > 0) {
      x--;
      stopping_x -= two_b_square;
      ellipse_error += x_change;
      x_change += two_b_square;
    }
  }

  x = 0;
  y = r_y;
  x_change = r_y * r_y;
  y_change = r_x * r_x * (1 - 2 * r_y);
  ellipse_error = 0;
  stopping_x = 0;
  stopping_y = two_a_square * r_y;

  // Drawing vertical portion of the ellipse
  while (stopping_x <= stopping_y) {
    last_v = QPoint(x, y);
    if (fill) {
      for (int i = 0; i < last_v.y(); i++) {
        Plot4EllipsePoints(image, c, QPoint(last_v.x(), i), e, color);
      }
    } else {
      Plot4EllipsePoints(image, c, last_v, e, color);
    }
    x++;
    stopping_x += two_b_square;
    ellipse_error += x_change;
    x_change += two_b_square;
    if ((2 * ellipse_error + y_change) > 0) {
      y--;
      stopping_y -= two_a_square;
      ellipse_error += y_change;
      y_change += two_a_square;
    }
  }

  // The two ellipse parts are separated and must be connected
  if (abs(last_h.x() - last_v.x()) > 1 || abs(last_h.y() - last_v.y()) > 1) {
    Bresenham4LinesEllipse(image, last_h, last_v, c, e, color);
  }
}

void ToolAlgorithm::Bresenham4LinesEllipse(QImage *image, const QPoint &p1, const QPoint &p2, const QPoint &c, const QPoint &e, const QRgb &color) {
  // Algorithm taken from http://www.roguebasin.com/index.php?title=Bresenham%27s_Line_Algorithm

  int x1 = p1.x();
  int y1 = p1.y();
  const int x2 = p2.x();
  const int y2 = p2.y();

  int delta_x(x2 - x1);
  signed char const ix((delta_x > 0) - (delta_x < 0));
  delta_x = std::abs(delta_x) << 1;

  int delta_y(y2 - y1);
  signed char const iy((delta_y > 0) - (delta_y < 0));
  delta_y = std::abs(delta_y) << 1;

  Plot4EllipsePoints(image, c, QPoint(x1, y1), e, color);

  if (delta_x >= delta_y) {
    int error(delta_y - (delta_x >> 1));
    while (x1 != x2) {
      if ((error >= 0) && (error || (ix > 0))) {
        error -= delta_x;
        y1 += iy;
      }
      error += delta_y;
      x1 += ix;

      Plot4EllipsePoints(image, c, QPoint(x1, y1), e, color);
    }
  } else {
    int error(delta_x - (delta_y >> 1));
    while (y1 != y2) {
      if ((error >= 0) && (error || (iy > 0))) {
        error -= delta_y;
        x1 += ix;
      }
      error += delta_x;
      y1 += iy;

      Plot4EllipsePoints(image, c, QPoint(x1, y1), e, color);
    }
  }
}

void ToolAlgorithm::Plot4EllipsePoints(QImage *image, const QPoint &c, const QPoint &p, const QPoint &e, const QRgb &color) {
  QPoint p1 = c + p;
  QPoint p2 = c - p + e;
  SetPixel(image, p1, color);
  SetPixel(image, p2, color);
  SetPixel(image, p1.x(), p2.y(), color);
  SetPixel(image, p2.x(), p1.y(), color);
}

void ToolAlgorithm::SetPixel(QImage *image, const QPoint &p, const QRgb &color) {
  if (image->rect().contains(p)) {
    image->setPixel(p, color);
  }
}

void ToolAlgorithm::SetPixel(QImage *image, const int x, const int y, const QRgb &color) {
  if (image->rect().contains(x, y)) {
    image->setPixel(x, y, color);
  }
}
