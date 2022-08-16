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
#include <QtGlobal>
#include <QSettings>
#include <QFile>
#include <QString>
#include <QRegExp>
#include <QVariant>
#include <QPushButton>
#include <QDebug>

#include <osmosdr/device.h>
#include <osmosdr/source.h>
#include <osmosdr/ranges.h>
#include <boost/foreach.hpp>

#ifdef WITH_PULSEAUDIO
#include "pulseaudio/pa_device_list.h"
#elif defined(WITH_PORTAUDIO)
#include "portaudio/device_list.h"
#endif

#include "qtgui/ioconfig.h"
#include "ui_ioconfig.h"


CIoConfig::CIoConfig(QSettings *settings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CIoConfig),
    m_settings(settings)
{
    unsigned int i=0;
    QString devstr;
    QString devlabel;
    bool cfgmatch=false; //flag to indicate that device from config was found

    ui->setupUi(this);

    QString indev = settings->value("input/device", "").toString();

    // automatic discovery of FCD does not work on Mac
    // so we do it ourselves if we have portaudio
#if defined(Q_WS_MAC) && defined(WITH_PORTAUDIO)
    portaudio_device_list devices;
    inDevList = devices.get_input_devices();

    string this_dev;
    for (i = 0; i < inDevList.size(); i++)
    {
        this_dev = inDevList[i].get_name();
        if (this_dev.find("FUNcube Dongle V1.0") != string::npos)
        {
            devstr = "fcd,type=1,device='FUNcube Dongle V1.0'";
            ui->inDevCombo->addItem("FUNcube Dongle V1.0", QVariant(devstr));
            
        }
        else if (this_dev.find("FUNcube Dongle V2.0") != string::npos)
        {
            devstr = "fcd,type=2,device='FUNcube Dongle V2.0'";
            ui->inDevCombo->addItem("FUNcube Dongle V2.0", QVariant(devstr));
        }

        if (indev == QString(inDevList[i].get_name().c_str()))
        {
            ui->inDevCombo->setCurrentIndex(i);
            ui->inDevEdit->setText(devstr);
            cfgmatch = true;
        }
    }
#endif

    // Get list of input devices discovered by gr-osmosdr and store them in
    // the input device selector together with the device descriptor strings
    osmosdr::devices_t devs = osmosdr::device::find();

    qDebug() << __FUNCTION__ << ": Available input devices:";
    BOOST_FOREACH(osmosdr::device_t &dev, devs)
    {
        if (dev.count("label"))
        {
            devlabel = QString(dev["label"].c_str());
            dev.erase("label");
        }
        else
        {
            devlabel = "Unknown";
        }

        devstr = QString(dev.to_string().c_str());
        ui->inDevCombo->addItem(devlabel, QVariant(devstr));

        // is this the device stored in config?
        if (indev == devstr)
        {
            ui->inDevCombo->setCurrentIndex(i);
            ui->inDevEdit->setText(devstr);
            cfgmatch = true;
        }

        qDebug() << "   " << i << ":"  << devlabel;
        ++i;

        // Following code could be used for multiple matches
        /* QStringList list;
        int pos = 0;
        while ((pos = rx.indexIn(devstr, pos)) != -1) {
            list << rx.cap(1);
            pos += rx.matchedLength();
        } */

    }

    ui->inDevCombo->addItem(tr("Other..."), QVariant(""));

    // If device string from config is not one of the detected devices
    // it could be that device is not plugged in (in which case we select
    // other) or that this is the first time (select the first detected device).
    if (!cfgmatch)
    {
        if (indev.isEmpty())
        {
            // First time config: select the first detected device
            ui->inDevCombo->setCurrentIndex(0);
            ui->inDevEdit->setText(ui->inDevCombo->itemData(0).toString());
            if (ui->inDevEdit->text().isEmpty())
                ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        }
        else
        {
            // Select other
            ui->inDevCombo->setCurrentIndex(i);
            ui->inDevEdit->setText(indev);
        }
    }

    updateInputSampleRates(settings->value("input/sample_rate", 0).toInt());

    // Analog bandwidth
    ui->bwSpinBox->setValue(1.0e-6*settings->value("input/bandwidth", 0.0).toDouble());

    // LNB LO
    ui->loSpinBox->setValue(1.0e-6*settings->value("input/lnb_lo", 0.0).toDouble());

    // Output device
    QString outdev = settings->value("output/device", "").toString();

#ifdef WITH_PULSEAUDIO
    // get list of output devices
    pa_device_list devices;
    outDevList = devices.get_output_devices();

    qDebug() << __FUNCTION__ << ": Available output devices:";
    for (i = 0; i < outDevList.size(); i++)
    {
        qDebug() << "   " << i << ":" << QString(outDevList[i].get_description().c_str());
        //qDebug() << "     " << QString(outDevList[i].get_name().c_str());
        ui->outDevCombo->addItem(QString(outDevList[i].get_description().c_str()));

        // note that item #i in devlist will be item #(i+1)
        // in combo box due to "default"
        if (outdev == QString(outDevList[i].get_name().c_str()))
            ui->outDevCombo->setCurrentIndex(i+1);
    }

#elif defined(Q_WS_MAC) && defined(WITH_PORTAUDIO)
    // get list of output devices
    // (already defined) portaudio_device_list devices;
    outDevList = devices.get_output_devices();

    qDebug() << __FUNCTION__ << ": Available output devices:";
    for (i = 0; i < outDevList.size(); i++)
    {
        qDebug() << "   " << i << ":" << QString(outDevList[i].get_name().c_str());
        ui->outDevCombo->addItem(QString(outDevList[i].get_name().c_str()));

        // note that item #i in devlist will be item #(i+1)
        // in combo box due to "default"
        if (outdev == QString(outDevList[i].get_name().c_str()))
            ui->outDevCombo->setCurrentIndex(i+1);
    }

#else
    ui->outDevCombo->setEditable(true);
#endif // WITH_PULSEAUDIO

    // Signals and slots
    connect(this, SIGNAL(accepted()), this, SLOT(saveConfig()));
    connect(ui->inDevCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(inputDeviceSelected(int)));
    connect(ui->inDevEdit, SIGNAL(textChanged(QString)), this, SLOT(inputDevstrChanged(QString)));
}

CIoConfig::~CIoConfig()
{
    delete ui;
}


/*! \brief Save configuration. */
void CIoConfig::saveConfig()
{
    qDebug() << __FUNCTION__;

    int idx = ui->outDevCombo->currentIndex();

    if (idx > 0)
    {
#if defined(WITH_PULSEAUDIO) || defined(WITH_PORTAUDIO)
        qDebug() << "Output device" << idx << ":" << QString(outDevList[idx-1].get_name().c_str());
        m_settings->setValue("output/device", QString(outDevList[idx-1].get_name().c_str()));
#endif
    }
    else
    {
        m_settings->remove("output/device");
    }

    // input settings
    m_settings->setValue("input/device", ui->inDevEdit->text());  // "OK" button disabled if empty

    qint64 value = (qint64)(ui->bwSpinBox->value()*1.e6);
    if (value)
        m_settings->setValue("input/bandwidth", value);
    else
        m_settings->remove("input/bandwidth");

    value = (qint64)(ui->loSpinBox->value()*1.e6);
    if (value)
        m_settings->setValue("input/lnb_lo", value);

    bool ok=false;
    int sr = ui->inSrCombo->currentText().toInt(&ok);
    if (ok)
        m_settings->setValue("input/sample_rate", sr);
    else
        m_settings->remove("input/sample_rate");

}


/*! \brief Update list of sample rates based on selected device.
 *  \param rate The current sample rate from the configuration.
 */
void CIoConfig::updateInputSampleRates(int rate)
{
    ui->inSrCombo->clear();

    if (ui->inDevEdit->text().isEmpty())
    {
        return;
    }

    /** FIXME: this code crashes on RTL device so we use fixed rates **/
    //osmosdr_source_c_sptr src = osmosdr_make_source_c(ui->inDevEdit->text().toStdString());
    //osmosdr::meta_range_t rates = src->get_sample_rates();
    //BOOST_FOREACH(osmosdr::range_t &rate, rates)
    //{
    //    ui->inSrCombo->addItem(QString("%1 kHz").arg(rate.start()/1000, 0, 'f', 0));
    //}
    //src.reset();

    if (ui->inDevEdit->text().contains("fcd"))
    {
        if (ui->inDevCombo->currentText().contains("V2.0"))
        {
            ui->inSrCombo->addItem("192000");
        }
        else
        {
            ui->inSrCombo->addItem("96000");
        }
    }
    else if (ui->inDevEdit->text().contains("rtl"))
    {
        ui->inSrCombo->addItem("250000");
        ui->inSrCombo->addItem("1200000");
        ui->inSrCombo->addItem("1500000");
        ui->inSrCombo->addItem("1700000");
        ui->inSrCombo->addItem("2000000");
        ui->inSrCombo->addItem("2200000");
        ui->inSrCombo->addItem("2400000");
        ui->inSrCombo->addItem("2700000");
        ui->inSrCombo->addItem("3200000");
        if (rate > 0)
        {
            ui->inSrCombo->addItem(QString("%1").arg(rate));
            ui->inSrCombo->setCurrentIndex(9);
        }
        else
        {
            ui->inSrCombo->setCurrentIndex(2);
        }
    }
    else if (ui->inDevEdit->text().contains("uhd"))
    {
        if (rate > 0)
            ui->inSrCombo->addItem(QString("%1").arg(rate));
        ui->inSrCombo->addItem("250000");
        ui->inSrCombo->addItem("500000");
        ui->inSrCombo->addItem("2000000");
        ui->inSrCombo->addItem("4000000");
        ui->inSrCombo->addItem("8000000");
    }
    else if (ui->inDevEdit->text().contains("hackrf"))
    {
        if (rate > 0)
            ui->inSrCombo->addItem(QString("%1").arg(rate));
        ui->inSrCombo->addItem("8000000");
        ui->inSrCombo->addItem("10000000");
        ui->inSrCombo->addItem("12500000");
        ui->inSrCombo->addItem("16000000");
        ui->inSrCombo->addItem("20000000");
    }
}


/*! \brief New input device has been selected by the user.
 *  \param index The index of the item that has been selected in the combo box.
 */
void CIoConfig::inputDeviceSelected(int index)
{
    qDebug() << "New input device selected:" << index;
    qDebug() << "  Label:" << ui->inDevCombo->itemText(index);
    qDebug() << "  Devstr:" << ui->inDevCombo->itemData(index).toString();

    ui->inDevEdit->setText(ui->inDevCombo->itemData(index).toString());
    updateInputSampleRates(0);
}


/*! \brief Input device string has changed
 *  \param text THe new device string
 *
 * This slot is activated when the device string in the text edit box has changed
 * either by the user or programatically. We use this to enable/disable the OK
 * button - we allo OK only if there is some text in the text entry.
 */
void CIoConfig::inputDevstrChanged(const QString &text)
{
    if (text.isEmpty())
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    else
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}
