#include "util.h"

#include <QWidget>
#include <QLayout>
#include <QLayoutItem>

void Util::clearLayout(QLayout *layout, int start)
{
    int items = layout->count();
    for (int i=start; i<items; ++i)
    {
        QLayoutItem *child = layout->takeAt(start);
        child->widget()->close();
        delete child->widget();
        delete child;
    }
}
