using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows.Forms;
using System.Globalization;
using ASCOM.Utilities;
using ASCOM.ZeGoto;

namespace ASCOM.ZeGoto
{
    [ComVisible(false)]					// Form not registered for COM!
    public partial class SetupDialogForm : Form
    {
        private Util utilities;

        public SetupDialogForm()
        {
            utilities = new Util();

            InitializeComponent();

            string[] s_Ports = System.IO.Ports.SerialPort.GetPortNames();

            comboSerialPort.Items.Add("Through OGCC");
            comboSerialPort.Items.Add("TCP/IP");
            comboSerialPort.Items.AddRange(s_Ports);

            if (Telescope.comPort != "")
            {
                int i_Port = 0;
                foreach (string p in comboSerialPort.Items)
                {
                    if (p == Telescope.comPort) break;
                    i_Port++;
                }
                if (i_Port == comboSerialPort.Items.Count) i_Port = comboSerialPort.Items.Add(Telescope.comPort);
                comboSerialPort.SelectedItem = Telescope.comPort;
            }
            textBoxIPAddress.Text = Telescope.ipAddress;
            textBoxIPPort.Text = Telescope.ipPort.ToString();

            textBoxAperture.Text = Telescope.apertureDiameter.ToString();
            if (textBoxAperture.Text == "") textBoxAperture.Text = "0";
            textBoxFocalLength.Text = Telescope.focalLength.ToString();
            if (textBoxFocalLength.Text == "") textBoxFocalLength.Text = "0";

            checkBoxUseGPS.Checked = Telescope.useGPS;
            if (Telescope.useGPS == true)
            {
                textBoxLatitude.Enabled = false;
                textBoxLongitude.Enabled = false;
            }

            textBoxLatitude.Text = utilities.DegreesToDMS(Telescope.latitude, "°", "'", "''", 0);
            if (Telescope.latitude >= 0.0)
                textBoxLatitude.Text += " N";
            else
                textBoxLatitude.Text += " S";

            textBoxLongitude.Text = utilities.DegreesToDMS(Telescope.longitude, "°", "'", "''", 0);
            if (Telescope.longitude >= 0.0)
                textBoxLongitude.Text += " E";
            else
                textBoxLongitude.Text += " W";

            textBoxElevation.Text = Telescope.elevation.ToString("###0");

            textBoxMaxRate.Text = Telescope.maxRate.ToString();
            comboBoxParkPosition.SelectedIndex = Telescope.parkPosition;

            labelVersion.Text = "v" + Application.ProductVersion;
            chkTrace.Checked = Telescope.traceState;
        }

        private void cmdOK_Click(object sender, EventArgs e) // OK button event handler
        {
            // Place any validation constraint checks here

            Telescope.comPort = comboSerialPort.GetItemText(comboSerialPort.SelectedItem);
            Telescope.ipAddress = textBoxIPAddress.Text;
            int.TryParse(textBoxIPPort.Text, out Telescope.ipPort);
            double.TryParse(textBoxAperture.Text, out Telescope.apertureDiameter);
            Telescope.apertureArea = 3.1415926535897932384626433832795 * Math.Pow(Telescope.apertureDiameter / 2, 2);
            double.TryParse(textBoxFocalLength.Text, out Telescope.focalLength);
            Telescope.useGPS = checkBoxUseGPS.Checked;
            Telescope.traceState = chkTrace.Checked;
            Telescope.parkPosition = comboBoxParkPosition.SelectedIndex;

            if (!checkBoxUseGPS.Checked)
            {
                if (textBoxLatitude.Text.IndexOf('°') > 0)
                {
                    Telescope.latitude = utilities.DMSToDegrees(textBoxLatitude.Text);
                }
                else
                {
                    double.TryParse(textBoxLatitude.Text, out Telescope.latitude);
                }

                if (textBoxLongitude.Text.IndexOf('°') > 0)
                {
                    Telescope.longitude = utilities.DMSToDegrees(textBoxLongitude.Text);
                }
                else
                {
                    double.TryParse(textBoxLatitude.Text, out Telescope.longitude);
                }

                double.TryParse(textBoxElevation.Text, out Telescope.elevation);
            }
        }

        private void cmdCancel_Click(object sender, EventArgs e) // Cancel button event handler
        {
            Close();
        }

        private void BrowseToAscom(object sender, EventArgs e) // Click on ASCOM logo event handler
        {
            try
            {
                System.Diagnostics.Process.Start("http://ascom-standards.org/");
            }
            catch (System.ComponentModel.Win32Exception noBrowser)
            {
                if (noBrowser.ErrorCode == -2147467259)
                    MessageBox.Show(noBrowser.Message);
            }
            catch (System.Exception other)
            {
                MessageBox.Show(other.Message);
            }
        }

        private void textBoxLatitude_TextChanged(object sender, EventArgs e)
        {
            if (textBoxLatitude.Text.IndexOf(CultureInfo.CurrentCulture.NumberFormat.NumberDecimalSeparator) >= 0 || textBoxLatitude.Text.IndexOf('.') >= 0)
                return;     // decimal input, no formating

            StringBuilder FormatedText = new StringBuilder(textBoxLatitude.Text);
            int seppos;
            if (textBoxLatitude.Text.IndexOf('°') < 0)
            {
                seppos = textBoxLatitude.Text.IndexOf(' ');
                if (seppos > 0) FormatedText[seppos] = '°';
            }
            else
            {
                if (textBoxLatitude.Text.IndexOf('\'') < 0)
                {
                    seppos = textBoxLatitude.Text.IndexOf(' ');
                    if (seppos > 0) FormatedText[seppos] = '\'';
                }
                else
                {
                    if (textBoxLatitude.Text.IndexOf("''") < 0)
                    {
                        seppos = textBoxLatitude.Text.IndexOf(' ');
                        if (seppos > 0)
                        {
                            FormatedText[seppos] = '\'';
                            FormatedText.Append("' ");
                        }
                    }
                }
            }

            textBoxLatitude.Text = FormatedText.ToString();
            textBoxLatitude.Select(textBoxLatitude.Text.Length, 0);
        }

        private void textBoxLongitude_TextChanged(object sender, EventArgs e)
        {
            if (textBoxLongitude.Text.IndexOf(CultureInfo.CurrentCulture.NumberFormat.NumberDecimalSeparator) >= 0 || textBoxLongitude.Text.IndexOf('.') >= 0)
                return;     // decimal input, no formating

            StringBuilder FormatedText = new StringBuilder(textBoxLongitude.Text);
            int seppos;
            if (textBoxLongitude.Text.IndexOf('°') < 0)
            {
                seppos = textBoxLongitude.Text.IndexOf(' ');
                if (seppos > 0) FormatedText[seppos] = '°';
            }
            else
            {
                if (textBoxLongitude.Text.IndexOf('\'') < 0)
                {
                    seppos = textBoxLongitude.Text.IndexOf(' ');
                    if (seppos > 0) FormatedText[seppos] = '\'';
                }
                else
                {
                    if (textBoxLongitude.Text.IndexOf("''") < 0)
                    {
                        seppos = textBoxLongitude.Text.IndexOf(' ');
                        if (seppos > 0)
                        {
                            FormatedText[seppos] = '\'';
                            FormatedText.Append("' ");
                        }
                    }
                }
            }

            textBoxLongitude.Text = FormatedText.ToString();
            textBoxLongitude.Select(textBoxLongitude.Text.Length, 0);
        }

        private void checkBox_UseGPS_CheckedChanged(object sender, EventArgs e)
        {
            textBoxLatitude.Enabled = !checkBoxUseGPS.Checked;
            textBoxLongitude.Enabled = !checkBoxUseGPS.Checked;
            textBoxElevation.Enabled = !checkBoxUseGPS.Checked;
        }

        private void comboSerialPort_SelectedIndexChanged(object sender, EventArgs e)
        {
            if ((string)comboSerialPort.SelectedItem == "TCP/IP")
            {
                textBoxIPAddress.Enabled = true;
                textBoxIPPort.Enabled = true;
            }
            else
            {
                textBoxIPAddress.Enabled = false;
                textBoxIPPort.Enabled = false;
            }
        }
    }
}