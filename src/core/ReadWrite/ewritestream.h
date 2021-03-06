#ifndef EWRITESTREAM_H
#define EWRITESTREAM_H

#include <QFile>

#include "efuturepos.h"

class SimpleBrushWrapper;
struct iValueRange;
class eWriteStream;

class eWriteFutureTable {
    friend class eWriteStream;
    eWriteFutureTable(QIODevice * const main) :
        mMain(main) {}

    void write(eWriteStream& dst);

    //! @brief Returns id to be used with assignFuturePos
    int planFuturePos() {
        const int id = mFutures.count();
        mFutures.append({-1, -1});
        return id;
    }

    void assignFuturePos(const int id) {
        mFutures.replace(id, {mMain->pos(), id});
    }
private:
    QList<eFuturePos> mFutures;
    QIODevice* const mMain;
};

class eWriteStream {
    friend class MainWindow;
public:
    class FuturePosId {
        friend class eWriteStream;
        FuturePosId(const int id) : fId(id) {}
        const int fId;
    };

    eWriteStream(QIODevice* const dst);

    void writeFutureTable();

    //! @brief Returns id to be used with assignFuturePos
    FuturePosId planFuturePos();

    void assignFuturePos(const FuturePosId id);

    void writeCheckpoint();

    qint64 writeFile(QFile* const file);

    inline qint64 write(const void* const data, const qint64 len) {
        return mDst->write(reinterpret_cast<const char*>(data), len);
    }

    eWriteStream& operator<<(const bool val);
    eWriteStream& operator<<(const int val);
    eWriteStream& operator<<(const uint val);
    eWriteStream& operator<<(const uint64_t val);
    eWriteStream& operator<<(const iValueRange val);
    eWriteStream& operator<<(const qreal val);
    eWriteStream& operator<<(const QPointF& val);
    eWriteStream& operator<<(const QMatrix& val);
    eWriteStream& operator<<(const QColor& val);
    eWriteStream& operator<<(const QString& val);
    eWriteStream& operator<<(SimpleBrushWrapper* const brush);
private:
    QIODevice* const mDst;
    eWriteFutureTable mFutureTable;
};
#endif // EWRITESTREAM_H
