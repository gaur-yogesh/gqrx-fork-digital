/* -*- c++ -*- */
/*
 * Gqrx SDR: Software defined radio receiver powered by GNU Radio and Qt
 *           http://gqrx.dk/
 *
 * Copyright 2011-2013 Alexandru Csete OZ9AEC.
 *
 * Gqrx is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * Gqrx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Gqrx; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */
#ifndef DOCKINPUTCTL_H
#define DOCKINPUTCTL_H

#include <vector>
#include <string>

#include <QDockWidget>
#include <QSettings>
#include <QString>

#include "qtgui/gain_options.h"

namespace Ui {
    class DockInputCtl;
}

class DockInputCtl : public QDockWidget
{
    Q_OBJECT

public:
    explicit DockInputCtl(QWidget *parent = 0);
    ~DockInputCtl();

    void readSettings(QSettings *settings);
    void saveSettings(QSettings *settings);

    void  setLnbLo(double freq_mhz);
    double lnbLo();

    void   setGain(double gain);
    double gain();

    void setFreqCorr(int corr);
    int  freqCorr();

    void setIqSwap(bool reversed);
    bool iqSwap(void);

    void setDcCancel(bool enabled);
    bool dcCancel(void);

    void setIqBalance(bool enabled);
    bool iqBalance(void);

    void setIgnoreLimits(bool reversed);
    bool ignoreLimits(void);

    void setAntennas(std::vector<std::string> &antennas);
    void setAntenna(const QString &antenna);

    void setGainStages(gain_list_t &gain_list);

signals:
    void gainChanged(double gain); /*!< Relative gain between 0.0 and 1.0 (negative means auto). */
    void freqCorrChanged(int value);
    void lnbLoChanged(double freq_mhz);
    void iqSwapChanged(bool reverse);
    void dcCancelChanged(bool enabled);
    void iqBalanceChanged(bool enabled);
    void ignoreLimitsChanged(bool ignore);
    void antennaSelected(QString antenna);

private slots:
    void on_lnbSpinBox_valueChanged(double value);
    void on_gainSlider_valueChanged(int value);
    void on_gainButton_toggled(bool checked);
    void on_gainOptButton_pressed();
    void on_freqCorrSpinBox_valueChanged(int value);
    void on_iqSwapButton_toggled(bool checked);
    void on_dcCancelButton_toggled(bool checked);
    void on_iqBalanceButton_toggled(bool checked);
    void on_ignoreButton_toggled(bool checked);
    void on_antSelector_currentIndexChanged(const QString &antenna);

private:
    Ui::DockInputCtl *ui;       /*!< User interface. */
    CGainOptions     *gainOpt;  /*!< Gain options popup. */
};

#endif // DOCKINPUTCTL_H
