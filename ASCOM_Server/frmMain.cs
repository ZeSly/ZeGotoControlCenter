using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace ASCOM.ZeGoto
{
    public partial class frmMain : Form
    {
        delegate void SetTextCallback(string text);

        public frmMain()
        {
            InitializeComponent();
            Server.OnConnect += new ConnectDelegate(OnClientConnect);
            SharedResources.OnConnect += new ConnectDelegate(OnLinkConnect);
        }

        public void OnClientConnect(bool Connected, int Clients)
        {
            label_nb_clients.Text = String.Format("{0} clients", Clients);
        }

        public void OnLinkConnect(bool Connected, int Clients)
        {
            if (Connected)
                label_connect.Text = String.Format("{0} connexion, connected", Clients);
            else
                label_connect.Text = String.Format("{0} connexion, disconnected", Clients);
        }
    }
}