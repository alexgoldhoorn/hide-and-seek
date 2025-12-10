#include "GUI/setparamsdialog.h"
#include "ui_setparamsdialog.h"
#include <QMessageBox>

const QStringList SetParamsDialog::PARAMS_STR_LIST = QStringList()
        << "Max game time (s)" //0
        << "Max game steps"
        << "Win distance"
        << "Num simulations"
        << "Num belief points"
        << "Sim. hider random act. prob."
        << "Next seeker state std.dev."
        << "Next hider state std.dev."
        << "Obs. seeker std.dev."
        << "Obs. hider std.dev."
        << "False positive prob." //10
        << "False negative prob."
        << "Minimum obstacle distance"
        << "Use next n poses (-tS)"
        << "Base hidden score (-tS)"
        << "Consist check seeker obs."
        << "Consist check hider obs."
        << "POMCP filter belief at update"
        << "Min. time between iterations (ms, -tS)"
        << "Hider step distance (grid units)"
        << "Seeker step distance (grid units)" //20
        << "Obs. if not visib prob."
        << "Incorrect positive obs. prob."
        << "Filter score type"
        << "Filter can stop number of iterations"
        << "Belief zoom factor (Highest Belief)";



SetParamsDialog::SetParamsDialog(SeekerHSParams* params, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetParamsDialog), _params(params)
{
    if (_params == NULL) {
        QMessageBox::warning(this,"WARNING","params is NULL, generating new one for testing purposes only!!");
        _params = new SeekerHSParams();
    }

    ui->setupUi(this);

    //set params
    ui->paramCombo->addItems(PARAMS_STR_LIST);

    //set slot to signal
    connect(ui->paramCombo, SIGNAL(currentIndexChanged(int)), SLOT(paramsComboIndexChanged(int)));
}

SetParamsDialog::~SetParamsDialog()
{
    delete ui;
}

void SetParamsDialog::paramsComboIndexChanged(int index) {
    //ui->valueEdit->setText(QString::number(index));
    switch (index) {
        case PARAMS_IDX_GAME_TIME:
            setValueText(_params->maxGameTime);
            break;
        case PARAMS_IDX_GAME_STEPS:
            setValueText(_params->maxNumActions);
            break;
        case PARAMS_IDX_WIN_DIST:
            setValueText(_params->winDist);
            break;
        case PARAMS_IDX_NUM_SIM:
            setValueText(_params->numSim);
            break;
        case PARAMS_IDX_NUM_BEL:
            setValueText(_params->numBeliefStates);
            break;
        case PARAMS_IDX_HIDER_RANDOM_ACT_PROB:
            setValueText(_params->simHiderRandomActProb);
            break;
        case PARAMS_IDX_CONT_NEXT_SS_SD:
            setValueText(_params->contNextSeekerStateStdDev);
            break;
        case PARAMS_IDX_CONT_NEXT_HS_SD:
            setValueText(_params->contNextHiderStateStdDev);
            break;
        case PARAMS_IDX_CONT_OBS_S_SD:
            setValueText(_params->contSeekerObsStdDev);
            break;
        case PARAMS_IDX_CONT_OBS_H_SD:
            setValueText(_params->contHiderObsStdDev);
            break;
        case PARAMS_IDX_CONT_FALSE_POS_P:
            setValueText(_params->contFalsePosProb);
            break;
        case PARAMS_IDX_CONT_FALSE_NEG_P:
            setValueText(_params->contFalseNegProb);
            break;
        case PARAMS_IDX_MIN_OBST_DIST:
            setValueText(_params->minDistToObstacle);
            break;
        case PARAMS_IDX_NEXT_N_POS:
            setValueText(_params->useNextNPos);
            break;
        case PARAMS_IDX_BASE_HIDDEN_OBS_SCORE:
            setValueText(_params->filterHiddenBaseScore);
            break;
        case PARAMS_IDX_CONSIST_CHECK_SEEKER_OBS:
            setValueText(_params->contConsistCheckSeekerDist);
            break;
        case PARAMS_IDX_CONSIST_CHECK_HIDER_OBS:
            setValueText(_params->contConsistCheckHiderDist);
            break;
        case PARAMS_IDX_FILTER_BELIEF_AT_UPDATE:
            setValueText(_params->pomcpFilterBeliefAtUpdate);
            break;
        case PARAMS_IDX_MIN_TIME_BETW_IT:
            setValueText((long)_params->minTimeBetweenIterations_ms);
            break;
        case PARAMS_IDX_SEEKER_STEP_DIST:
            setValueText(_params->seekerStepDistance);
            break;
        case PARAMS_IDX_HIDER_STEP_DIST:
            setValueText(_params->hiderStepDistance);
            break;
        case PARAMS_IDX_OBS_IF_NOT_VISIB_PROB:
            setValueText(_params->contObserveIfNotVisibProb);
            break;
        case PARAMS_IDX_INCOR_POS_PROB:
            setValueText(_params->contIncorPosProb);
            break;
        case PARAMS_IDX_FILTER_CAN_STOP_NUM_OF_IT:
            setValueText(_params->filterCanStopNumberOfIterations);
            break;
        case PARAMS_IDX_FILTER_SCORE_TYPE:
            setValueText(_params->filterScoreType);
            break;
        case PARAMS_IDX_BELIEF_ZOOM_FAC:
            setValueText(_params->beliefMapZoomFactor);
            break;
    }
}

void SetParamsDialog::on_setButton_clicked()
{
    //ui->valueEdit->setText(QString::number(index));
    QString valText;

    switch (ui->paramCombo->currentIndex()) {
    case PARAMS_IDX_GAME_TIME:
        _params->maxGameTime = getValueAsLong(_params->maxGameTime);
        break;
    case PARAMS_IDX_GAME_STEPS:
        _params->maxNumActions = getValueAsLong(_params->maxNumActions);
        break;
    case PARAMS_IDX_NUM_SIM:
        _params->numSim = getValueAsUInt(_params->numSim);
        break;
    case PARAMS_IDX_WIN_DIST:
        _params->winDist = getValueAsDouble(_params->winDist);
        break;
    case PARAMS_IDX_NUM_BEL:
        _params->numBeliefStates = getValueAsUInt(_params->numBeliefStates);
        break;
    case PARAMS_IDX_HIDER_RANDOM_ACT_PROB:
        _params->simHiderRandomActProb = getValueAsDouble(_params->simHiderRandomActProb);
        break;
    case PARAMS_IDX_CONT_NEXT_SS_SD:
        _params->contNextSeekerStateStdDev = getValueAsDouble(_params->contNextSeekerStateStdDev);
        break;
    case PARAMS_IDX_CONT_NEXT_HS_SD:
        _params->contNextHiderStateStdDev = getValueAsDouble(_params->contNextHiderStateStdDev);
        break;
    case PARAMS_IDX_CONT_OBS_S_SD:
        _params->contSeekerObsStdDev = getValueAsDouble(_params->contSeekerObsStdDev);
        break;
    case PARAMS_IDX_CONT_OBS_H_SD:
        _params->contHiderObsStdDev = getValueAsDouble(_params->contHiderObsStdDev);
        break;
    case PARAMS_IDX_CONT_FALSE_POS_P:
        _params->contFalsePosProb = getValueAsDouble(_params->contFalsePosProb);
        break;
    case PARAMS_IDX_CONT_FALSE_NEG_P:
        _params->contFalseNegProb = getValueAsDouble(_params->contFalseNegProb);
        break;
    case PARAMS_IDX_MIN_OBST_DIST:
        _params->minDistToObstacle = getValueAsDouble(_params->minDistToObstacle);
        break;
    case PARAMS_IDX_NEXT_N_POS:
        _params->useNextNPos = getValueAsInt(_params->useNextNPos);
        break;
    case PARAMS_IDX_BASE_HIDDEN_OBS_SCORE:
        _params->filterHiddenBaseScore = getValueAsDouble(_params->filterHiddenBaseScore);
        break;
    case PARAMS_IDX_CONSIST_CHECK_SEEKER_OBS:
        _params->contConsistCheckSeekerDist = getValueAsDouble(_params->contConsistCheckSeekerDist);
        break;
    case PARAMS_IDX_CONSIST_CHECK_HIDER_OBS:
        _params->contConsistCheckHiderDist = getValueAsDouble(_params->contConsistCheckHiderDist);
        break;
    case PARAMS_IDX_FILTER_BELIEF_AT_UPDATE:
        _params->pomcpFilterBeliefAtUpdate = getValueAsBool(_params->pomcpFilterBeliefAtUpdate);
        break;
    case PARAMS_IDX_MIN_TIME_BETW_IT:
        _params->minTimeBetweenIterations_ms = getValueAsLong(_params->minTimeBetweenIterations_ms);
        break;
    case PARAMS_IDX_HIDER_STEP_DIST:
        _params->hiderStepDistance = getValueAsDouble(_params->hiderStepDistance);
        break;
    case PARAMS_IDX_SEEKER_STEP_DIST:
        _params->seekerStepDistance = getValueAsDouble(_params->seekerStepDistance);
        break;
    case PARAMS_IDX_OBS_IF_NOT_VISIB_PROB:
        _params->contObserveIfNotVisibProb = getValueAsDouble(_params->contObserveIfNotVisibProb);
        break;
    case PARAMS_IDX_INCOR_POS_PROB:
        _params->contIncorPosProb = getValueAsDouble(_params->contIncorPosProb);
        break;
    case PARAMS_IDX_FILTER_CAN_STOP_NUM_OF_IT:
        _params->filterCanStopNumberOfIterations = getValueAsUInt(_params->filterCanStopNumberOfIterations);
        break;
    case PARAMS_IDX_FILTER_SCORE_TYPE:
        _params->filterScoreType = getValueAsInt(_params->filterScoreType);
        break;
    case PARAMS_IDX_BELIEF_ZOOM_FAC:
        _params->beliefMapZoomFactor = getValueAsDouble(_params->beliefMapZoomFactor);
        break;
    }
}

void SetParamsDialog::on_buttonBox_accepted()
{
    on_setButton_clicked();
    close();
}

void SetParamsDialog::on_buttonBox_rejected()
{
    close();
}


void SetParamsDialog::setValueText(unsigned int v) {
    setValueText((long)v);
}
void SetParamsDialog::setValueText(int v) {
    setValueText((long)v);
}
void SetParamsDialog::setValueText(bool v) {
    QString valText = (v?"true":"false");
    ui->valueEdit->setText(valText);
}
void SetParamsDialog::setValueText(long v) {
    QString valText = QString::number(v);
    ui->valueEdit->setText(valText);
}

void SetParamsDialog::setValueText(double v) {
    QString valText = QString::number(v);
    ui->valueEdit->setText(valText);
}

int SetParamsDialog::getValueAsInt(int v) {
    bool ok = false;
    QString val = ui->valueEdit->text();
    int nv = val.toInt(&ok);
    if (!ok) {
        QMessageBox::warning(this,"Error converting value","Could not convert the value to int.");
        return v;
    } else {
        return nv;
    }
}

unsigned int SetParamsDialog::getValueAsUInt(unsigned int v) {
    bool ok = false;
    QString val = ui->valueEdit->text();
    unsigned int nv = val.toUInt(&ok);
    if (!ok) {
        QMessageBox::warning(this,"Error converting value","Could not convert the value to unsigned int.");
        return v;
    } else {
        return nv;
    }
}
long SetParamsDialog::getValueAsLong(long v) {
    bool ok = false;
    QString val = ui->valueEdit->text();
    long nv = val.toLong(&ok);
    if (!ok) {
        QMessageBox::warning(this,"Error converting value","Could not convert the value to long.");
        return v;
    } else {
        return nv;
    }
}
double SetParamsDialog::getValueAsDouble(double v) {
    bool ok = false;
    QString val = ui->valueEdit->text();
    double nv = val.toDouble(&ok);
    if (!ok) {
        QMessageBox::warning(this,"Error converting value","Could not convert the value to double.");
        return v;
    } else {
        return nv;
    }
}
bool SetParamsDialog::getValueAsBool(bool v) {
    bool ok = false;
    QString val = ui->valueEdit->text().trimmed().toLower();

    if (val.compare("true")==0 || val.compare("yes")) {
        return true;
    } else if (val.compare("false")==0 || val.compare("no")) {
        return true;
    } else {
        QMessageBox::warning(this,"Error converting value","Could not convert the value to bool.");
        return v;
    }
}
