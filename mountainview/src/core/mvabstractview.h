/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/15/2016
*******************************************************/

#ifndef MVABSTRACTVIEW_H
#define MVABSTRACTVIEW_H

#include "mvcontext.h"
#include "mlutils.h"
#include <QMimeData>
#include <QWidget>

class MVAbstractViewFactory;
class MVAbstractViewPrivate;
class CalculationThread;
class MVAbstractView : public QWidget {
    Q_OBJECT
public:
    enum ViewFeature {
        NoFeatures = 0x0,
        RenderView = 0x1 // can render itself on a given painter (e.g. for export)
    };
    Q_DECLARE_FLAGS(ViewFeatures, ViewFeature)

    friend class MVAbstractViewPrivate;
    friend class CalculationThread;
    MVAbstractView(MVContext* context);
    virtual ~MVAbstractView();

    bool isCalculating() const;
    bool recalculateSuggested() const;
    void stopCalculation();

public slots:
    void recalculate();
    void neverSuggestRecalculate();

    virtual MVAbstractViewFactory* viewFactory() const;
    MVContext* mvContext();

    virtual ViewFeatures viewFeatures() const;
    virtual void renderView(QPainter* painter); // add render opts

signals:
    void calculationStarted();
    void calculationFinished();
    void recalculateSuggestedChanged();
    void signalClusterContextMenu();
    void contextMenuRequested(const QMimeData&, const QPoint& globalPos);

protected:
    /*
     * prepareCalculation() and onCalculationFinished() are called in the gui thread,
     * whereas runCalculation() is called in the worker thread.
     * Therefore, it is important that runCalculation() does not access any
     * variables that may be used in the gui thread. However, it is guaranteed
     * that prepareCalculation() and onCalculationFinished() will not be called
     * in while runCalculation() is running. The calculation should check periodically
     * whether thread_interrupt_requested() is true. In that case, it is
     * important to return from runCalculation() prematurely.
     */
    virtual void prepareCalculation() = 0;
    virtual void runCalculation() = 0;
    virtual void onCalculationFinished() = 0;

    void recalculateOnOptionChanged(QString name, bool suggest_only = true);
    void recalculateOn(QObject*, const char* signal, bool suggest_only = true);

    void requestContextMenu(const QMimeData& md, const QPoint& pos);
    virtual void prepareMimeData(QMimeData& mimeData, const QPoint& pos);

    void contextMenuEvent(QContextMenuEvent* evt);
private slots:
    void slot_do_calculation();
    void slot_calculation_finished();
    void slot_context_option_changed(QString name);
    void slot_suggest_recalculate();

private:
    MVAbstractViewPrivate* d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(MVAbstractView::ViewFeatures)

#endif // MVABSTRACTVIEW_H
