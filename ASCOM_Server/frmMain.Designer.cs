using System;

namespace ASCOM.ZeGoto
{
    partial class frmMain
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.label_nb_clients = new System.Windows.Forms.Label();
            this.label_connect = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // label_nb_clients
            // 
            this.label_nb_clients.AutoSize = true;
            this.label_nb_clients.Location = new System.Drawing.Point(12, 10);
            this.label_nb_clients.Name = "label_nb_clients";
            this.label_nb_clients.Size = new System.Drawing.Size(95, 13);
            this.label_nb_clients.TabIndex = 0;
            this.label_nb_clients.Text = "0 client connected";
            // 
            // label_connect
            // 
            this.label_connect.AutoSize = true;
            this.label_connect.Location = new System.Drawing.Point(12, 30);
            this.label_connect.Name = "label_connect";
            this.label_connect.Size = new System.Drawing.Size(0, 13);
            this.label_connect.TabIndex = 1;
            // 
            // frmMain
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(233, 52);
            this.Controls.Add(this.label_connect);
            this.Controls.Add(this.label_nb_clients);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.Name = "frmMain";
            this.Text = "ZeGoto Driver Server";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label_nb_clients;
        private System.Windows.Forms.Label label_connect;

    }
}

