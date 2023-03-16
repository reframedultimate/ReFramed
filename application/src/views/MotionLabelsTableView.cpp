#include "application/views/MotionLabelsTableView.hpp"

#include "rfcommon/Profiler.hpp"

#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>

namespace rfapp {

// ----------------------------------------------------------------------------
MotionLabelsTableView::MotionLabelsTableView(QWidget* parent)
    : QTableView(parent)
{
}

// ----------------------------------------------------------------------------
void MotionLabelsTableView::keyPressEvent(QKeyEvent* e)
{
    PROFILE(TableView, keyPressEvent);

    // If Ctrl-C typed
    // Or use event->matches(QKeySequence::Copy)
    if (e->key() == Qt::Key_C && (e->modifiers() & Qt::ControlModifier))
    {
        QModelIndexList cells = selectedIndexes();
        std::sort(cells.begin(), cells.end()); // Necessary, otherwise they are in column order

        QString text;
        int currentRow = 0; // To determine when to insert newlines
        foreach(const QModelIndex & cell, cells) {

            if (text.length() == 0) {
                // First item
            }
            else if (cell.row() != currentRow) {
                // New row
                text += '\n';
            }
            else {
                // Next cell
                text += '\t';
            }
            currentRow = cell.row();
            text += cell.data().toString();
        }

        QApplication::clipboard()->setText(text);
    }
}

// ----------------------------------------------------------------------------
void MotionLabelsTableView::dropEvent(QDropEvent* e)
{
    if (e->source() != this || e->dropAction() != Qt::MoveAction)
        return;
}

}
