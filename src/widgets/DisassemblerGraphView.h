#ifndef DISASSEMBLERGRAPHVIEW_H
#define DISASSEMBLERGRAPHVIEW_H

// Based on the DisassemblerGraphView from x64dbg

#include <QWidget>
#include <QPainter>
#include <QShortcut>
#include <QLabel>

#include "widgets/GraphView.h"
#include "menus/DisassemblyContextMenu.h"
#include "common/RichTextPainter.h"
#include "common/CutterSeekable.h"

class QTextEdit;
class SyntaxHighlighter;

class DisassemblerGraphView : public GraphView
{
    Q_OBJECT

    struct Text {
        std::vector<RichTextPainter::List> lines;

        Text() {}

        Text(const QString &text, QColor color, QColor background)
        {
            RichTextPainter::List richText;
            RichTextPainter::CustomRichText_t rt;
            rt.highlight = false;
            rt.text = text;
            rt.textColor = color;
            rt.textBackground = background;
            rt.flags = rt.textBackground.alpha() ? RichTextPainter::FlagAll : RichTextPainter::FlagColor;
            richText.push_back(rt);
            lines.push_back(richText);
        }

        Text(const RichTextPainter::List &richText)
        {
            lines.push_back(richText);
        }

        QString ToQString() const
        {
            QString result;
            for (const auto &line : lines) {
                for (const auto &t : line) {
                    result += t.text;
                }
            }
            return result;
        }
    };

    struct Instr {
        ut64 addr = 0;
        ut64 size = 0;
        Text text;
        Text fullText;
        QString plainText;
        std::vector<unsigned char> opcode; //instruction bytes
    };

    struct Token {
        int start;
        int length;
        QString type;
        Instr *instr;
        QString name;
        QString content;
    };

    struct DisassemblyBlock {
        Text header_text;
        std::vector<Instr> instrs;
        ut64 entry = 0;
        ut64 true_path = 0;
        ut64 false_path = 0;
        bool terminal = false;
        bool indirectcall = false;
    };

public:
    DisassemblerGraphView(QWidget *parent);
    ~DisassemblerGraphView() override;
    std::unordered_map<ut64, DisassemblyBlock> disassembly_blocks;
    virtual void drawBlock(QPainter &p, GraphView::GraphBlock &block) override;
    virtual void blockClicked(GraphView::GraphBlock &block, QMouseEvent *event, QPoint pos) override;
    virtual void blockDoubleClicked(GraphView::GraphBlock &block, QMouseEvent *event,
                                    QPoint pos) override;
    virtual bool helpEvent(QHelpEvent *event) override;
    virtual void blockHelpEvent(GraphView::GraphBlock &block, QHelpEvent *event, QPoint pos) override;
    virtual GraphView::EdgeConfiguration edgeConfiguration(GraphView::GraphBlock &from,
                                                           GraphView::GraphBlock *to) override;
    virtual void blockTransitionedTo(GraphView::GraphBlock *to) override;

    void loadCurrentGraph();
    QString windowTitle;
    QTextEdit *header = nullptr;

    int getWidth() { return width; }
    int getHeight() { return height; }
    std::unordered_map<ut64, GraphBlock> getBlocks() { return blocks; }

public slots:
    void refreshView();
    void colorsUpdatedSlot();
    void fontsUpdatedSlot();
    void onSeekChanged(RVA addr);
    void toggleSync();

    void zoom(QPointF mouseRelativePos, double velocity);
    void zoomReset();

    void takeTrue();
    void takeFalse();

    void nextInstr();
    void prevInstr();

    void copySelection();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private slots:
    void on_actionExportGraph_triggered();

private:
    bool transition_dont_seek = false;

    Token *highlight_token;
    // Font data
    CachedFontMetrics *mFontMetrics;
    qreal charWidth;
    int charHeight;
    int charOffset;
    int baseline;
    bool emptyGraph;

    DisassemblyContextMenu *mMenu;

    void connectSeekChanged(bool disconnect);

    void initFont();
    void prepareGraphNode(GraphBlock &block);
    void prepareHeader();
    Token *getToken(Instr *instr, int x);
    RVA getAddrForMouseEvent(GraphBlock &block, QPoint *point);
    Instr *getInstrForMouseEvent(GraphBlock &block, QPoint *point);
    DisassemblyBlock *blockForAddress(RVA addr);
    void seekLocal(RVA addr, bool update_viewport = true);
    void seekInstruction(bool previous_instr);
    CutterSeekable *seekable = nullptr;
    QList<QShortcut *> shortcuts;
    QList<RVA> breakpoints;

    QColor disassemblyBackgroundColor;
    QColor disassemblySelectedBackgroundColor;
    QColor disassemblySelectionColor;
    QColor PCSelectionColor;
    QColor jmpColor;
    QColor brtrueColor;
    QColor brfalseColor;
    QColor retShadowColor;
    QColor indirectcallShadowColor;
    QColor mAutoCommentColor;
    QColor mAutoCommentBackgroundColor;
    QColor mCommentColor;
    QColor mCommentBackgroundColor;
    QColor mLabelColor;
    QColor mLabelBackgroundColor;
    QColor graphNodeColor;
    QColor mAddressColor;
    QColor mAddressBackgroundColor;
    QColor mCipColor;
    QColor mBreakpointColor;
    QColor mDisabledBreakpointColor;

    QAction actionExportGraph;
    QAction actionSyncOffset;

    QLabel *emptyText = nullptr;
    SyntaxHighlighter *highlighter = nullptr;

signals:
    void viewRefreshed();
    void viewZoomed();
    void graphMoved();
};

#endif // DISASSEMBLERGRAPHVIEW_H
