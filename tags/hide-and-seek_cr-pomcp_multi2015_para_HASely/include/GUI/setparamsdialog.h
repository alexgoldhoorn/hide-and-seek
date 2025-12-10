#ifndef SETPARAMSDIALOG_H
#define SETPARAMSDIALOG_H

#include <QDialog>
#include <QStringList>

#include "Base/seekerhsparams.h"

namespace Ui {
class SetParamsDialog;
}

class SetParamsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SetParamsDialog(SeekerHSParams* params, QWidget *parent = 0);
    ~SetParamsDialog();

private:
    static const QStringList PARAMS_STR_LIST;


    static const int PARAMS_IDX_GAME_TIME = 0;
    static const int PARAMS_IDX_GAME_STEPS = 1;
    static const int PARAMS_IDX_WIN_DIST = 2;
    static const int PARAMS_IDX_NUM_SIM = 3;
    static const int PARAMS_IDX_NUM_BEL = 4;
    static const int PARAMS_IDX_HIDER_RANDOM_ACT_PROB = 5;
    static const int PARAMS_IDX_CONT_NEXT_SS_SD = 6;
    static const int PARAMS_IDX_CONT_NEXT_HS_SD = 7;
    static const int PARAMS_IDX_CONT_OBS_S_SD = 8;
    static const int PARAMS_IDX_CONT_OBS_H_SD = 9;
    static const int PARAMS_IDX_CONT_FALSE_POS_P = 10;
    static const int PARAMS_IDX_CONT_FALSE_NEG_P = 11;
    static const int PARAMS_IDX_MIN_OBST_DIST = 12;
    static const int PARAMS_IDX_NEXT_N_POS = 13;
    static const int PARAMS_IDX_BASE_HIDDEN_OBS_SCORE = 14;
    static const int PARAMS_IDX_CONSIST_CHECK_SEEKER_OBS = 15;
    static const int PARAMS_IDX_CONSIST_CHECK_HIDER_OBS = 16;
    static const int PARAMS_IDX_FILTER_BELIEF_AT_UPDATE = 17;
    static const int PARAMS_IDX_MIN_TIME_BETW_IT = 18;
    static const int PARAMS_IDX_HIDER_STEP_DIST = 19;
    static const int PARAMS_IDX_SEEKER_STEP_DIST = 20;
    static const int PARAMS_IDX_OBS_IF_NOT_VISIB_PROB = 21;
    static const int PARAMS_IDX_INCOR_POS_PROB = 22;
    static const int PARAMS_IDX_FILTER_SCORE_TYPE = 23;
    static const int PARAMS_IDX_FILTER_CAN_STOP_NUM_OF_IT = 24;
    static const int PARAMS_IDX_BELIEF_ZOOM_FAC = 25;


    Ui::SetParamsDialog *ui;
    SeekerHSParams* _params;

    //! set value of text box
    void setValueText(long v);
    void setValueText(unsigned int v);
    void setValueText(int v);
    void setValueText(double v);
    void setValueText(bool v);

    //!get value
    int getValueAsInt(int v);
    unsigned int getValueAsUInt(unsigned int v);
    long getValueAsLong(long v);
    double getValueAsDouble(double v);
    bool getValueAsBool(bool v);

private slots:
    void paramsComboIndexChanged(int index);

    void on_setButton_clicked();
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
};

#endif // SETPARAMSDIALOG_H
