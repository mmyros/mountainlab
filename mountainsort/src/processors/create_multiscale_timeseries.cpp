#include "create_multiscale_timeseries.h"
#include "cachemanager.h"
#include <QStringList>
#include <QFile>
#include <QTime>
#include "diskreadmda.h"
#include "diskwritemda.h"

long smallest_power_of_3_larger_than(long N);
bool downsample_min(const DiskReadMda& X, QString out_fname, long N);
bool downsample_max(const DiskReadMda& X, QString out_fname, long N);
bool write_concatenation(QStringList input_fnames, QString output_fname);

bool create_multiscale_timeseries(QString path_in, QString path_out)
{
    DiskReadMda X(path_in);

    long N = smallest_power_of_3_larger_than(X.N2());

    QStringList file_names;
    QString prev_min_fname;
    QString prev_max_fname;
    for (long ds_factor = 3; ds_factor <= N; ds_factor *= 3) {
        printf("ds_factor = %ld / %ld\n", ds_factor, N);
        QString min_fname = CacheManager::globalInstance()->makeLocalFile("", CacheManager::ShortTerm);
        QString max_fname = CacheManager::globalInstance()->makeLocalFile("", CacheManager::ShortTerm);
        if (ds_factor == 3) {
            if (!downsample_min(X, min_fname, N)) {
                printf("Problem in downsample_min\n");
                return false;
            }
            if (!downsample_max(X, max_fname, N)) {
                printf("Problem in downsample_max\n");
                return false;
            }
        } else {
            if (!downsample_min(DiskReadMda(prev_min_fname), min_fname, (N * 3) / ds_factor)) {
                printf("Problem in downsample_min *\n");
                return false;
            }
            if (!downsample_max(DiskReadMda(prev_max_fname), max_fname, (N * 3) / ds_factor)) {
                printf("Problem in downsample_max *\n");
                return false;
            }
        }
        file_names << min_fname;
        file_names << max_fname;
        prev_min_fname = min_fname;
        prev_max_fname = max_fname;
    }

    if (!write_concatenation(file_names, path_out)) {
        printf("Problem in write_concatenation\n");
        return false;
    }
    printf("Removing temporary files\n");
    foreach(QString fname, file_names)
    {
        QFile::remove(fname);
    }

    return true;
}

long smallest_power_of_3_larger_than(long N)
{
    long ret = 1;
    while (ret < N) {
        ret *= 3;
    }
    return ret;
}

bool downsample_min(const DiskReadMda& X, QString out_fname, long N)
{
    DiskWriteMda Y;
    if (!Y.open(MDAIO_TYPE_FLOAT32, out_fname, X.N1(), N / 3)) {
        qWarning() << "Unable to open output file in downsample_min: " + out_fname;
        return false;
    }

    /// TODO choose chunk_size sensibly
    long chunk_size = 3 * 5000;
    QTime timer;
    timer.start();
    for (long ii = 0; ii < N; ii += chunk_size) {
        if (timer.elapsed() > 5000) {
            printf("downsample_min %ld/%ld (%d%%)\n", ii, N, (int)(ii * 1.0 / N * 100));
            timer.restart();
        }
        long size0 = qMin(chunk_size, N - ii);
        Mda A;
        X.readChunk(A, 0, ii, X.N1(), size0);
        Mda B(X.N1(), size0 / 3);
        for (long jj = 0; jj < size0 / 3; jj++) {
            for (long m = 0; m < X.N1(); m++) {
                double val1 = A.value(m, jj * 3);
                double val2 = A.value(m, jj * 3 + 1);
                double val3 = A.value(m, jj * 3 + 2);
                double val = qMin(qMin(val1, val2), val3);
                B.setValue(val, m, jj);
            }
        }
        Y.writeChunk(B, 0, ii / 3);
    }

    return true;
}

bool downsample_max(const DiskReadMda& X, QString out_fname, long N)
{
    DiskWriteMda Y;
    if (!Y.open(MDAIO_TYPE_FLOAT32, out_fname, X.N1(), N / 3)) {
        qWarning() << "Unable to open output file in downsample_min: " + out_fname;
        return false;
    }

    /// TODO choose chunk_size sensibly
    long chunk_size = 3 * 5000;
    QTime timer;
    timer.start();
    for (long ii = 0; ii < N; ii += chunk_size) {
        if (timer.elapsed() > 5000) {
            printf("downsample_max %ld/%ld (%d%%)\n", ii, N, (int)(ii * 1.0 / N * 100));
            timer.restart();
        }
        long size0 = qMin(chunk_size, N - ii);
        Mda A;
        X.readChunk(A, 0, ii, X.N1(), size0);
        Mda B(X.N1(), size0 / 3);
        for (long jj = 0; jj < size0 / 3; jj++) {
            for (long m = 0; m < X.N1(); m++) {
                double val1 = A.value(m, jj * 3);
                double val2 = A.value(m, jj * 3 + 1);
                double val3 = A.value(m, jj * 3 + 2);
                double val = qMax(qMax(val1, val2), val3);
                B.setValue(val, m, jj);
            }
        }
        Y.writeChunk(B, 0, ii / 3);
    }

    return true;
}

bool write_concatenation(QStringList input_fnames, QString output_fname)
{
    long M = 1, N = 0;
    foreach(QString fname, input_fnames)
    {
        DiskReadMda X(fname);
        M = X.N1();
        N += X.N2();
    }
    DiskWriteMda Y;
    if (!Y.open(MDAIO_TYPE_FLOAT32, output_fname, M, N)) {
        qWarning() << "Unable to open output file: " + output_fname;
        return false;
    }
    long offset = 0;
    foreach(QString fname, input_fnames)
    {
        DiskReadMda X(fname);

        /// TODO choose chunk_size sensibly
        long chunk_size = 10000;
        for (long ii=0; ii<X.N2(); ii+=chunk_size) {
            long size0 = qMin(chunk_size, X.N2() - ii);
            Mda tmp;
            X.readChunk(tmp, 0, ii, M, size0);
            Y.writeChunk(tmp, 0, offset);
            offset += X.N2();
        }
    }
    Y.close();

    return true;
}
