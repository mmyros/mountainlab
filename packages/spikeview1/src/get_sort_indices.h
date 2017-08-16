/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef GET_SORT_INDICES_H
#define GET_SORT_INDICES_H

#include <QList>
#include "mlcommon.h"
#include "mlvector.h"

MLVector<bigint> get_sort_indices(const MLVector<double>& X);

//QList<int> get_sort_indices(const QList<int>& X);
//QVector<int> get_sort_indices(const QVector<int>& X);
//QList<int> get_sort_indices(const QVector<double>& X);
//QList<bigint> get_sort_indices_bigint(const QVector<double>& X);

#endif // GET_SORT_INDICES_H
