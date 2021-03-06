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

#include "selection_tool.h"

#include "utils/debug.h"
#include "logic/undo_redo.h"

#include <QPainter>

void SelectionTool::Use(QImage *image, QRect *selection, QImage *image_selected, const QColor &color, QPoint *anchor, bool *started, const ToolEvent &event) {
  if (event.action() == ACTION_PRESS) {
    if (event.lmb_down()) {
      if (selection->isValid() && selection->contains(event.img_pos())) {
        // Selection already exists. Moving it.
        *anchor = event.img_pos() - selection->center();
      } else {
        // Selection do not exist. Creating it.
        ClearSelection(image, selection, image_selected);
        *anchor = event.img_pos();
        *started = true;
        *selection = GetRect(*anchor, event.img_pos());
      }
    } else {
      // Pressing rmb clears the selection.
      ClearSelection(image, selection, image_selected);
    }
  } else if (event.action() == ACTION_MOVE) {
    if (*started) {
      // Creating the selection area.
      *selection = GetRect(*anchor, event.img_pos());
    } else {
      // Selection area already exists. Move it.
      if (selection->isValid() && event.lmb_down()) {
        selection->moveCenter(event.img_pos() - *anchor);
      }
    }
  } else if (event.action() == ACTION_RELEASE) {
    if (*started == true) {
      event.undo_redo()->Do(*image);
      *image_selected = image->copy(*selection);
      QPainter p(image);
      p.fillRect(*selection, color);
    }
    *started = false;
  }
}

QRect SelectionTool::GetRect(const QPoint &start, const QPoint &end) {
  return QRect(
      qMin(start.x(), end.x()),
      qMin(start.y(), end.y()),
      qAbs(start.x() - end.x()) + 1,
      qAbs(start.y() - end.y()) + 1);
}

void SelectionTool::ClearSelection(QImage *image, QRect *selection, QImage *image_selected) {
  if (!image_selected->isNull()) {
    QPainter p(image);
    p.drawImage(*selection, *image_selected);
  }
  *image_selected = QImage();
  *selection = QRect();
}
