namespace ASCOM.ZeGoto
{
    partial class SetupDialogForm
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
            this.cmdOK = new System.Windows.Forms.Button();
            this.cmdCancel = new System.Windows.Forms.Button();
            this.picASCOM = new System.Windows.Forms.PictureBox();
            this.groupBoxTelescopeGeometry = new System.Windows.Forms.GroupBox();
            this.label_mmFoc = new System.Windows.Forms.Label();
            this.textBoxFocalLength = new System.Windows.Forms.TextBox();
            this.labelFocalLength = new System.Windows.Forms.Label();
            this.label_mmAp = new System.Windows.Forms.Label();
            this.textBoxAperture = new System.Windows.Forms.TextBox();
            this.labelAperture = new System.Windows.Forms.Label();
            this.groupBoxSiteLocation = new System.Windows.Forms.GroupBox();
            this.checkBoxUseGPS = new System.Windows.Forms.CheckBox();
            this.label_mElev = new System.Windows.Forms.Label();
            this.textBoxElevation = new System.Windows.Forms.TextBox();
            this.labelElevation = new System.Windows.Forms.Label();
            this.textBoxLongitude = new System.Windows.Forms.TextBox();
            this.labelLongitude = new System.Windows.Forms.Label();
            this.textBoxLatitude = new System.Windows.Forms.TextBox();
            this.labelLatitude = new System.Windows.Forms.Label();
            this.chkTrace = new System.Windows.Forms.CheckBox();
            this.labelVersion = new System.Windows.Forms.Label();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            ((System.ComponentModel.ISupportInitialize)(this.picASCOM)).BeginInit();
            this.groupBoxTelescopeGeometry.SuspendLayout();
            this.groupBoxSiteLocation.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.SuspendLayout();
            // 
            // cmdOK
            // 
            this.cmdOK.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.cmdOK.BackColor = System.Drawing.SystemColors.Control;
            this.cmdOK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.cmdOK.ForeColor = System.Drawing.SystemColors.ControlText;
            this.cmdOK.Location = new System.Drawing.Point(343, 214);
            this.cmdOK.Name = "cmdOK";
            this.cmdOK.Size = new System.Drawing.Size(75, 23);
            this.cmdOK.TabIndex = 0;
            this.cmdOK.Text = "OK";
            this.cmdOK.UseVisualStyleBackColor = false;
            this.cmdOK.Click += new System.EventHandler(this.cmdOK_Click);
            // 
            // cmdCancel
            // 
            this.cmdCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.cmdCancel.BackColor = System.Drawing.SystemColors.Control;
            this.cmdCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.cmdCancel.ForeColor = System.Drawing.SystemColors.ControlText;
            this.cmdCancel.Location = new System.Drawing.Point(262, 214);
            this.cmdCancel.Name = "cmdCancel";
            this.cmdCancel.Size = new System.Drawing.Size(75, 23);
            this.cmdCancel.TabIndex = 1;
            this.cmdCancel.Text = "Cancel";
            this.cmdCancel.UseVisualStyleBackColor = false;
            this.cmdCancel.Click += new System.EventHandler(this.cmdCancel_Click);
            // 
            // picASCOM
            // 
            this.picASCOM.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.picASCOM.Cursor = System.Windows.Forms.Cursors.Hand;
            this.picASCOM.Image = global::ASCOM.ZeGoto.Properties.Resources.ASCOM;
            this.picASCOM.Location = new System.Drawing.Point(370, 12);
            this.picASCOM.Name = "picASCOM";
            this.picASCOM.Size = new System.Drawing.Size(48, 56);
            this.picASCOM.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
            this.picASCOM.TabIndex = 3;
            this.picASCOM.TabStop = false;
            this.picASCOM.Click += new System.EventHandler(this.BrowseToAscom);
            this.picASCOM.DoubleClick += new System.EventHandler(this.BrowseToAscom);
            // 
            // groupBoxTelescopeGeometry
            // 
            this.groupBoxTelescopeGeometry.Controls.Add(this.label_mmFoc);
            this.groupBoxTelescopeGeometry.Controls.Add(this.textBoxFocalLength);
            this.groupBoxTelescopeGeometry.Controls.Add(this.labelFocalLength);
            this.groupBoxTelescopeGeometry.Controls.Add(this.label_mmAp);
            this.groupBoxTelescopeGeometry.Controls.Add(this.textBoxAperture);
            this.groupBoxTelescopeGeometry.Controls.Add(this.labelAperture);
            this.groupBoxTelescopeGeometry.ForeColor = System.Drawing.Color.White;
            this.groupBoxTelescopeGeometry.Location = new System.Drawing.Point(15, 74);
            this.groupBoxTelescopeGeometry.Name = "groupBoxTelescopeGeometry";
            this.groupBoxTelescopeGeometry.Size = new System.Drawing.Size(200, 75);
            this.groupBoxTelescopeGeometry.TabIndex = 38;
            this.groupBoxTelescopeGeometry.TabStop = false;
            this.groupBoxTelescopeGeometry.Text = "Telescope geometry";
            // 
            // label_mmFoc
            // 
            this.label_mmFoc.AutoSize = true;
            this.label_mmFoc.Location = new System.Drawing.Point(161, 48);
            this.label_mmFoc.Name = "label_mmFoc";
            this.label_mmFoc.Size = new System.Drawing.Size(23, 13);
            this.label_mmFoc.TabIndex = 21;
            this.label_mmFoc.Text = "mm";
            // 
            // textBoxFocalLength
            // 
            this.textBoxFocalLength.Location = new System.Drawing.Point(85, 45);
            this.textBoxFocalLength.Name = "textBoxFocalLength";
            this.textBoxFocalLength.Size = new System.Drawing.Size(70, 20);
            this.textBoxFocalLength.TabIndex = 20;
            // 
            // labelFocalLength
            // 
            this.labelFocalLength.AutoSize = true;
            this.labelFocalLength.Location = new System.Drawing.Point(8, 48);
            this.labelFocalLength.Name = "labelFocalLength";
            this.labelFocalLength.Size = new System.Drawing.Size(65, 13);
            this.labelFocalLength.TabIndex = 19;
            this.labelFocalLength.Text = "Focal length";
            // 
            // label_mmAp
            // 
            this.label_mmAp.AutoSize = true;
            this.label_mmAp.Location = new System.Drawing.Point(161, 22);
            this.label_mmAp.Name = "label_mmAp";
            this.label_mmAp.Size = new System.Drawing.Size(23, 13);
            this.label_mmAp.TabIndex = 18;
            this.label_mmAp.Text = "mm";
            // 
            // textBoxAperture
            // 
            this.textBoxAperture.Location = new System.Drawing.Point(85, 19);
            this.textBoxAperture.Name = "textBoxAperture";
            this.textBoxAperture.Size = new System.Drawing.Size(70, 20);
            this.textBoxAperture.TabIndex = 18;
            // 
            // labelAperture
            // 
            this.labelAperture.AutoSize = true;
            this.labelAperture.Location = new System.Drawing.Point(7, 22);
            this.labelAperture.Name = "labelAperture";
            this.labelAperture.Size = new System.Drawing.Size(47, 13);
            this.labelAperture.TabIndex = 18;
            this.labelAperture.Text = "Aperture";
            // 
            // groupBoxSiteLocation
            // 
            this.groupBoxSiteLocation.Controls.Add(this.checkBoxUseGPS);
            this.groupBoxSiteLocation.Controls.Add(this.label_mElev);
            this.groupBoxSiteLocation.Controls.Add(this.textBoxElevation);
            this.groupBoxSiteLocation.Controls.Add(this.labelElevation);
            this.groupBoxSiteLocation.Controls.Add(this.textBoxLongitude);
            this.groupBoxSiteLocation.Controls.Add(this.labelLongitude);
            this.groupBoxSiteLocation.Controls.Add(this.textBoxLatitude);
            this.groupBoxSiteLocation.Controls.Add(this.labelLatitude);
            this.groupBoxSiteLocation.ForeColor = System.Drawing.Color.White;
            this.groupBoxSiteLocation.Location = new System.Drawing.Point(221, 74);
            this.groupBoxSiteLocation.Name = "groupBoxSiteLocation";
            this.groupBoxSiteLocation.Size = new System.Drawing.Size(197, 126);
            this.groupBoxSiteLocation.TabIndex = 39;
            this.groupBoxSiteLocation.TabStop = false;
            this.groupBoxSiteLocation.Text = "Site location";
            // 
            // checkBoxUseGPS
            // 
            this.checkBoxUseGPS.AutoSize = true;
            this.checkBoxUseGPS.Location = new System.Drawing.Point(11, 20);
            this.checkBoxUseGPS.Name = "checkBoxUseGPS";
            this.checkBoxUseGPS.Size = new System.Drawing.Size(119, 17);
            this.checkBoxUseGPS.TabIndex = 25;
            this.checkBoxUseGPS.Text = "Use telescope GPS";
            this.checkBoxUseGPS.UseVisualStyleBackColor = true;
            this.checkBoxUseGPS.CheckedChanged += new System.EventHandler(this.checkBox_UseGPS_CheckedChanged);
            // 
            // label_mElev
            // 
            this.label_mElev.AutoSize = true;
            this.label_mElev.Location = new System.Drawing.Point(161, 99);
            this.label_mElev.Name = "label_mElev";
            this.label_mElev.Size = new System.Drawing.Size(15, 13);
            this.label_mElev.TabIndex = 24;
            this.label_mElev.Text = "m";
            // 
            // textBoxElevation
            // 
            this.textBoxElevation.Location = new System.Drawing.Point(85, 96);
            this.textBoxElevation.Name = "textBoxElevation";
            this.textBoxElevation.Size = new System.Drawing.Size(70, 20);
            this.textBoxElevation.TabIndex = 23;
            // 
            // labelElevation
            // 
            this.labelElevation.AutoSize = true;
            this.labelElevation.Location = new System.Drawing.Point(8, 99);
            this.labelElevation.Name = "labelElevation";
            this.labelElevation.Size = new System.Drawing.Size(51, 13);
            this.labelElevation.TabIndex = 22;
            this.labelElevation.Text = "Elevation";
            // 
            // textBoxLongitude
            // 
            this.textBoxLongitude.Location = new System.Drawing.Point(85, 70);
            this.textBoxLongitude.Name = "textBoxLongitude";
            this.textBoxLongitude.Size = new System.Drawing.Size(70, 20);
            this.textBoxLongitude.TabIndex = 20;
            this.textBoxLongitude.TextChanged += new System.EventHandler(this.textBoxLongitude_TextChanged);
            // 
            // labelLongitude
            // 
            this.labelLongitude.AutoSize = true;
            this.labelLongitude.Location = new System.Drawing.Point(8, 73);
            this.labelLongitude.Name = "labelLongitude";
            this.labelLongitude.Size = new System.Drawing.Size(54, 13);
            this.labelLongitude.TabIndex = 19;
            this.labelLongitude.Text = "Longitude";
            // 
            // textBoxLatitude
            // 
            this.textBoxLatitude.Location = new System.Drawing.Point(85, 44);
            this.textBoxLatitude.Name = "textBoxLatitude";
            this.textBoxLatitude.Size = new System.Drawing.Size(70, 20);
            this.textBoxLatitude.TabIndex = 18;
            this.textBoxLatitude.TextChanged += new System.EventHandler(this.textBoxLatitude_TextChanged);
            // 
            // labelLatitude
            // 
            this.labelLatitude.AutoSize = true;
            this.labelLatitude.Location = new System.Drawing.Point(7, 47);
            this.labelLatitude.Name = "labelLatitude";
            this.labelLatitude.Size = new System.Drawing.Size(45, 13);
            this.labelLatitude.TabIndex = 18;
            this.labelLatitude.Text = "Latitude";
            // 
            // chkTrace
            // 
            this.chkTrace.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.chkTrace.AutoSize = true;
            this.chkTrace.Location = new System.Drawing.Point(15, 207);
            this.chkTrace.Name = "chkTrace";
            this.chkTrace.Size = new System.Drawing.Size(73, 17);
            this.chkTrace.TabIndex = 40;
            this.chkTrace.Text = "Trace ON";
            this.chkTrace.UseVisualStyleBackColor = true;
            // 
            // labelVersion
            // 
            this.labelVersion.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.labelVersion.AutoSize = true;
            this.labelVersion.Location = new System.Drawing.Point(12, 227);
            this.labelVersion.Name = "labelVersion";
            this.labelVersion.Size = new System.Drawing.Size(41, 13);
            this.labelVersion.TabIndex = 41;
            this.labelVersion.Text = "version";
            // 
            // pictureBox1
            // 
            this.pictureBox1.Image = global::ASCOM.ZeGoto.Properties.Resources.opengoto;
            this.pictureBox1.Location = new System.Drawing.Point(13, 12);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(204, 47);
            this.pictureBox1.TabIndex = 42;
            this.pictureBox1.TabStop = false;
            // 
            // SetupDialogForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.Black;
            this.ClientSize = new System.Drawing.Size(430, 249);
            this.Controls.Add(this.pictureBox1);
            this.Controls.Add(this.labelVersion);
            this.Controls.Add(this.chkTrace);
            this.Controls.Add(this.groupBoxSiteLocation);
            this.Controls.Add(this.groupBoxTelescopeGeometry);
            this.Controls.Add(this.picASCOM);
            this.Controls.Add(this.cmdCancel);
            this.Controls.Add(this.cmdOK);
            this.ForeColor = System.Drawing.Color.White;
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "SetupDialogForm";
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "ZeGoto Setup";
            ((System.ComponentModel.ISupportInitialize)(this.picASCOM)).EndInit();
            this.groupBoxTelescopeGeometry.ResumeLayout(false);
            this.groupBoxTelescopeGeometry.PerformLayout();
            this.groupBoxSiteLocation.ResumeLayout(false);
            this.groupBoxSiteLocation.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button cmdOK;
        private System.Windows.Forms.Button cmdCancel;
        private System.Windows.Forms.PictureBox picASCOM;
        private System.Windows.Forms.GroupBox groupBoxTelescopeGeometry;
        private System.Windows.Forms.Label label_mmFoc;
        private System.Windows.Forms.TextBox textBoxFocalLength;
        private System.Windows.Forms.Label labelFocalLength;
        private System.Windows.Forms.Label label_mmAp;
        private System.Windows.Forms.TextBox textBoxAperture;
        private System.Windows.Forms.Label labelAperture;
        private System.Windows.Forms.GroupBox groupBoxSiteLocation;
        private System.Windows.Forms.CheckBox checkBoxUseGPS;
        private System.Windows.Forms.Label label_mElev;
        private System.Windows.Forms.TextBox textBoxElevation;
        private System.Windows.Forms.Label labelElevation;
        private System.Windows.Forms.TextBox textBoxLongitude;
        private System.Windows.Forms.Label labelLongitude;
        private System.Windows.Forms.TextBox textBoxLatitude;
        private System.Windows.Forms.Label labelLatitude;
        private System.Windows.Forms.CheckBox chkTrace;
        private System.Windows.Forms.Label labelVersion;
        private System.Windows.Forms.PictureBox pictureBox1;
    }
}