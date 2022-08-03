//tabs=4
// --------------------------------------------------------------------------------
// TODO fill in this information for your driver, then remove this line!
//
// ASCOM Telescope driver for ZeGoto
//
// Description:	Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam 
//				nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam 
//				erat, sed diam voluptua. At vero eos et accusam et justo duo 
//				dolores et ea rebum. Stet clita kasd gubergren, no sea takimata 
//				sanctus est Lorem ipsum dolor sit amet.
//
// Implements:	ASCOM Telescope interface version: <To be completed by driver developer>
// Author:		(XXX) Your N. Here <your@email.here>
//
// Edit Log:
//
// Date			Who	Vers	Description
// -----------	---	-----	-------------------------------------------------------
// dd-mmm-yyyy	XXX	6.0.0	Initial edit, created from ASCOM driver template
// --------------------------------------------------------------------------------
//


// This is used to define code in the template that is specific to one class implementation
// unused code canbe deleted and this definition removed.
#define Telescope

using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using ASCOM;
using ASCOM.Astrometry;
using ASCOM.Utilities;
using ASCOM.DeviceInterface;
using System.Globalization;
using System.Collections;
using System.Text.RegularExpressions;
using System.Reflection;
using System.Configuration;
using System.IO;
using System.Diagnostics;

namespace ASCOM.ZeGoto
{
    //
    // Your driver's DeviceID is ASCOM.ZeGoto.Telescope
    //
    // The Guid attribute sets the CLSID for ASCOM.ZeGoto.Telescope
    // The ClassInterface/None addribute prevents an empty interface called
    // _ZeGoto from being created and used as the [default] interface
    //
    // TODO Replace the not implemented exceptions with code to implement the function or
    // throw the appropriate ASCOM exception.
    //

    /// <summary>
    /// ASCOM Telescope Driver for ZeGoto.
    /// </summary>
    [Guid("eb03dcc9-cb9c-4054-bf40-fb0ee70d20c1")]
    [ProgId("ASCOM.ZeGoto.Telescope")]
    [ServedClassName("ZeGoto")]
    [ClassInterface(ClassInterfaceType.None)]
    public partial class Telescope : ReferenceCountedObjectBase, ITelescopeV3
    {
        [DllImport("kernel32.dll")]
        private static extern int GetTickCount();

        private static readonly Object lockObject = new object();

        /// <summary>
        /// ASCOM DeviceID (COM ProgID) for this driver.
        /// The DeviceID is used by ASCOM applications to load the driver at runtime.
        /// </summary>
        internal static string driverID;// = "ASCOM.ZeGoto.Telescope";
        // TODO Change the descriptive string for your driver then remove this line
        /// <summary>
        /// Driver description that displays in the ASCOM Chooser.
        /// </summary>
        private static string driverDescription = "ZeGoto";

        internal static string traceStateProfileName = "Trace Level";
        internal static string traceStateDefault = "true";

        // Variables to hold the currrent device configuration
        internal static double apertureArea;
        internal static double apertureDiameter;
        internal static double focalLength;

        internal static bool useGPS;
        internal static double latitude;
        internal static double longitude;
        internal static double elevation;

        internal static double maxRate;

        internal static bool traceState;

        /// <summary>
        /// Private variable to hold an ASCOM Utilities object
        /// </summary>
        private Util utilities;

        /// <summary>
        /// Private variable to hold the trace logger object (creates a diagnostic log file with information that you specify)
        /// </summary>
        private TraceLogger tl;

        /// <summary>
        /// Private variable to hold the SharedResources.SharedLink to ZeGoto through serial port or TCP/IP socket
        /// </summary>
        //private SerialOrSocket SharedResources.SharedLink;

        //
        // Constants
        //
        public static string MSG_NOT_IMPLEMENTED = " is not implemented in this driver.";
        public static string MSG_NOT_CONNECTED = "The scope is not connected.";
        public static string MSG_NO_SCOPE = "There doesn't appear to be anything connected to the COM port.";
        // This uses runtime-generated messages
        public static string MSG_SYNC_FAILED = "The sync operation failed for some reason.";
        public static string MSG_PROP_RANGE_ERROR = "The supplied value is out of range for this property.";
        // This uses runtime-generated messages
        public static string MSG_BELOW_HORIZ = "The selected coordinates are below the horizon.";
        public static string MSG_BELOW_MIN_EL = "The selected coordinates are below the current minimum elevation setting.";
        public static string MSG_SLEWING = "The mount is slewin.";
        public static string MSG_PARKED = "The mount is parked.";
        public static string MSG_SLEW_FAIL = "The slew failed for some unknown reason.";
        public static string MSG_SETUP_CONNECTED = "You cannot change the driver's configuration while it is connected to a telescope.";
        public static string MSG_PROPNOTSET = " property has not yet been set.";
        public static string MSG_TGTSETERR = "Controller rejected set ";

        //
        // State Variables
        //
        private double m_dPrevRA;
        private double m_dPrevDec;
        private double m_dRARes;           // degrees 
        private double m_dDecRes;          // degrees 
        private bool m_bStartingSlew;    // Super ugly!
        private int m_lAsyncEndTix;
        private short m_iSettleTime;
        private double m_dTargetRA;
        private bool m_bTargetRAValid;
        private double m_dTargetDec;
        private bool m_bTargetDecValid;

        /// <summary>
        /// Initializes a new instance of the <see cref="ZeGoto"/> class.
        /// Must be public for COM registration.
        /// </summary>
        public Telescope()
        {
            driverID = Marshal.GenerateProgIdForType(this.GetType());

            ReadProfile(); // Read device configuration from the ASCOM Profile store

            //#if DEBUG
            //            string d = Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location) + @"\ASCOM.ZeGoto.log";
            //            tl = new TraceLogger(d, "ZeGoto");
            //#else
            tl = new TraceLogger("ZeGoto");
            //#endif
            tl.Enabled = traceState;
            tl.LogMessage("Telescope", "Starting initialisation " + System.Reflection.Assembly.GetExecutingAssembly().Location);

            utilities = new Util(); //Initialise util object
            //SharedResources.SharedLink = new SerialOrSocket();

            SharedResources.isParked = false;
            tl.LogMessage("Telescope", "Completed initialisation");
        }


        //
        // PUBLIC COM INTERFACE ITelescopeV3 IMPLEMENTATION
        //

        #region Common properties and methods.

        /// <summary>
        /// Displays the Setup Dialog form.
        /// If the user clicks the OK button to dismiss the form, then
        /// the new settings are saved, otherwise the old values are reloaded.
        /// THIS IS THE ONLY PLACE WHERE SHOWING USER INTERFACE IS ALLOWED!
        /// </summary>
        public void SetupDialog()
        {
            // consider only showing the setup dialog if not connected
            // or call a different dialog if connected
            if (SharedResources.Connected)
            {
                System.Windows.Forms.MessageBox.Show("Already connected, just press OK");
            }
            else
            {
                using (SetupDialogForm F = new SetupDialogForm())
                {
                    var result = F.ShowDialog();
                    if (result == System.Windows.Forms.DialogResult.OK)
                    {
                        WriteProfile(); // Persist device configuration values to the ASCOM Profile store
                    }
                }
            }
        }

        public ArrayList SupportedActions
        {
            get
            {
                tl.LogMessage("SupportedActions Get", "Returning empty arraylist");
                return new ArrayList();
            }
        }

        public string Action(string actionName, string actionParameters)
        {
            throw new ASCOM.ActionNotImplementedException("Action " + actionName + " is not implemented by this driver");
        }

        public void CommandBlind(string command, bool raw)
        {
            try
            {
                CheckConnected("CommandBlind");
                SharedResources.SharedLink.ClearBuffers();                    // Clear remaining junk in buffer
                if (!raw)
                    SharedResources.SharedLink.Transmit(":" + command + "#");
                else
                    SharedResources.SharedLink.Transmit(command);
            }
            catch (Exception e)
            {
                SharedResources.SharedLink.ClearBuffers();
                //SharedResources.Connected = false;  // Release the port
                tl.LogMessage("CommandBlind", "Exception command=" + command + " raw=" + raw.ToString());
                tl.LogMessage("CommandBlind", "Exception.Message: " + e.Message);
                throw e;
            }
        }

        public bool CommandBool(string command, bool raw)
        {
            string ret;

            try
            {
                lock (lockObject)
                {
                    CheckConnected("CommandBool");
                    SharedResources.SharedLink.ClearBuffers();    // Clear remaining junk in buffer
                    if (!raw)
                        SharedResources.SharedLink.Transmit(":" + command + "#");
                    else
                        SharedResources.SharedLink.Transmit(command);
                    ret = SharedResources.SharedLink.ReceiveCounted(1);   // Just a 1 or 0

                    return (ret == "1");
                }
            }
            catch (Exception e)
            {
                SharedResources.SharedLink.ClearBuffers();
                //SharedResources.Connected = false;  // Release the port
                tl.LogMessage("CommandBool", "Exception command=" + command + " raw=" + raw.ToString());
                tl.LogMessage("CommandBool", "Exception.Message: " + e.Message);
                throw e;
            }
        }

        public string CommandString(string command, bool raw)
        {
            try
            {
                lock (lockObject)
                {

                    CheckConnected("CommandString");
                    // it's a good idea to put all the low level communication with the device here,
                    // then all communication calls this function
                    // you need something to ensure that only one command is in progress at a time

                    string buf;
                    string ret = "";

                    SharedResources.SharedLink.ClearBuffers();
                    if (!raw)
                    {
                        SharedResources.SharedLink.Transmit(":" + command + "#");
                        buf = SharedResources.SharedLink.ReceiveTerminated("#");
                        if (buf != "")  // Overflow protection
                        {
                            ret = buf.Substring(0, buf.Length - 1); // Strip '#'
                        }
                    }
                    else
                    {
                        SharedResources.SharedLink.Transmit(command);
                        buf = SharedResources.SharedLink.Receive();
                        ret = buf;
                    }

                    return ret;
                }
            }
            catch (Exception e)
            {
                SharedResources.SharedLink.ClearBuffers();
                //SharedResources.Connected = false;  // Release the port
                tl.LogMessage("CommandString", "Exception command=" + command + " raw=" + raw.ToString());
                tl.LogMessage("CommandString", "Exception.Message: " + e.Message);
                throw e;
            }
        }

        public void Dispose()
        {
            this.Connected = false;

            // Clean up the tracelogger and util objects
            tl.Enabled = false;
            tl.Dispose();
            tl = null;
            utilities.Dispose();
            utilities = null;
        }

        Process ZeGotoControlCenterProcess;

        public bool Connected
        {
            get
            {
                tl.LogMessage("Connected Get", SharedResources.Connected.ToString());
                return SharedResources.Connected;
            }
            set
            {
                tl.LogMessage("Connected Set", value.ToString());
                if (value)
                {
                    string sRA, sDec;
                    Regex rx;
                    MatchCollection mt;

                    tl.LogMessage("Connected Set", "Connecting...");

                    // 1) Set up the communications SharedResources.SharedLink.
                    if (!SharedResources.Connected)
                    {
                        tl.LogMessage("Connected Set", "Connecting... ");
                        string d = Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location);
                        string ZeGotoControlCenterPath = d + @"\ZeGotoControlCenter.exe";

                        try
                        {
                            ProcessStartInfo startInfo = new ProcessStartInfo(ZeGotoControlCenterPath);
                            startInfo.WindowStyle = ProcessWindowStyle.Minimized;
                            startInfo.Arguments = "-minimized";
                            ZeGotoControlCenterProcess = Process.Start(startInfo);
                        }
                        catch (Exception e)
                        {
                            SharedResources.Connected = false;
                            tl.LogMessage("Connected Set", d);
                            tl.LogMessage("Connected Set", ZeGotoControlCenterPath + " : " + e.Message);
                            Exception myexp = new Exception(ZeGotoControlCenterPath + " : " + e.Message);
                            throw myexp;
                        }

                        SharedResources.SharedLink.IPAddress = "127.0.0.1";
                        SharedResources.SharedLink.Port = 5085;
                        SharedResources.SharedLink.TcpIp = true;
                        SharedResources.SharedLink.ReceiveTimeout = 5;
                    }
                    SharedResources.Connected = true;

                    // 2) Determine that there is an LX200 scope there, initialize
                    //     some variables for below...
                    try
                    {
                        tl.LogMessage("Connected Set", "sending GR...");
                        sRA = this.CommandString("GR", false);
                        tl.LogMessage("Connected Set", "GR transmited, sending GD...");
                        m_dPrevRA = utilities.HMSToHours(sRA);
                        sDec = this.CommandString("GD", false);
                        tl.LogMessage("Connected Set", "GD transmited, sending rM");
                        m_dPrevDec = utilities.DMSToDegrees(sDec);
                        string sMaxRate = CommandString("rM", false);
                        tl.LogMessage("Connected Set", "rM transmited, Max rate=" + sMaxRate);
                        maxRate = Convert.ToDouble(sMaxRate);
                    }
                    catch (Exception e)
                    {
                        SharedResources.SharedLink.ClearBuffers();
                        SharedResources.Connected = false;  // Release the port
                        tl.LogMessage("Connected Set", "Error: No response from ZeGotoControlCenter !");
                        throw new NotConnectedException(e.Message + "\r\nNo response from ZeGotoControlCenter !");
                    }
                    SharedResources.SharedLink.ClearBuffers();        // Toss any junk remaining in buffers

                    // 3) Final timeout & try for long format.
                    SharedResources.SharedLink.ReceiveTimeout = 10;   // Switch to 10 sec timeout
                    SharedResources.LongFormat = true;
                    //
                    // (4) Initialize slew detection parameters. The registry values may be
                    //     missing if the SetupDialog() had not yet been used. We default
                    //     to 60 arcsec. Avoid use of decimal values for international use.
                    //
                    SharedResources.Slewing = false;
                    SharedResources.AbortSlew = false;
                    m_dRARes = 60 / 3600;
                    m_dDecRes = 60 / 3600;
                    //
                    // (5) Get the delimiters that this implementation uses. Re-read the
                    //     RA and Dec now that we might have Long Format set!
                    //
                    sRA = this.CommandString("GR", false);
                    m_dPrevRA = utilities.HMSToHours(sRA);
                    sDec = this.CommandString("GD", false);
                    m_dPrevDec = utilities.DMSToDegrees(sDec);
                    rx = new Regex(@"[\+\-]*[0-9\.]+([^0-9\.]+)[0-9\.]*([^0-9\.]*)[0-9\.]*([^0-9\.]*)");
                    mt = rx.Matches(sRA);
                    SharedResources.RADelimHrs = DelimTrim(mt[0].Groups[1].Value);       // Hours delimiter
                    if (SharedResources.LongFormat)
                    {
                        SharedResources.RADelimMin = DelimTrim(mt[0].Groups[2].Value);   // Minutes delimiter
                        SharedResources.RADelimSec = mt[0].Groups[3].Value.Trim();       // Don't append trailing blanks
                    }
                    else
                    {
                        SharedResources.RADelimMin = mt[0].Groups[2].Value.Trim();        // Don't append trailing blanks
                        SharedResources.RADelimSec = "";
                    }
                    mt = rx.Matches(sDec);
                    SharedResources.DecDelimDeg = DelimTrim(mt[0].Groups[1].Value);       // Hours delimiter
                    if (SharedResources.LongFormat)
                    {
                        SharedResources.DecDelimMin = DelimTrim(mt[0].Groups[2].Value);   // Minutes delimiter
                        SharedResources.DecDelimSec = mt[0].Groups[3].Value.Trim();       // Don't append trailing blanks
                    }
                    else
                    {
                        SharedResources.DecDelimMin = mt[0].Groups[2].Value.Trim();        // Don't append trailing blanks
                        SharedResources.DecDelimSec = "";
                    }
                    
                    tl.LogMessage("Connected Set", "Connected. nb connections=" + SharedResources.connections.ToString());
                }
                else
                {
                    tl.LogMessage("Connected Set", "Disconnecting... nb connections=" + SharedResources.connections.ToString());

                    SharedResources.SharedLink.ClearBuffers();    // Clear serial buffers
                    SharedResources.Connected = false;

                    if (SharedResources.Connected)
                        tl.LogMessage("Connected Set", "Still connected. nb connections=" + SharedResources.connections.ToString());
                    else
                        tl.LogMessage("Connected Set", "Disconnected. nb connections=" + SharedResources.connections.ToString());
                }
            }
        }

        public string Description
        {
            // TODO customise this device description
            get
            {
                tl.LogMessage("Description Get", driverDescription);
                return driverDescription;
            }
        }

        public string DriverInfo
        {
            get
            {
                //
                // Use the Project/Properties sheet, Make tab, to set these
                // items. That way they will show in the Version tab of the
                // Explorer property sheet, and the exact same data will
                // show in Telescope.DriverInfo.
                //
                Assembly a = Assembly.GetExecutingAssembly();
                AssemblyName an = a.GetName();
                AssemblyTitleAttribute objTitle = (AssemblyTitleAttribute)AssemblyTitleAttribute.GetCustomAttribute(a, typeof(AssemblyTitleAttribute));
                AssemblyCompanyAttribute objCompany = (AssemblyCompanyAttribute)AssemblyCompanyAttribute.GetCustomAttribute(a, typeof(AssemblyCompanyAttribute));
                AssemblyCopyrightAttribute objCopyright = (AssemblyCopyrightAttribute)AssemblyCopyrightAttribute.GetCustomAttribute(a, typeof(AssemblyCopyrightAttribute));
                AssemblyDescriptionAttribute objDescription = (AssemblyDescriptionAttribute)AssemblyDescriptionAttribute.GetCustomAttribute(a, typeof(AssemblyDescriptionAttribute));
                string sDriverInfo = objTitle.Title + " " + an.Version.Major + "." + an.Version.Minor + "." + an.Version.Revision;
                if (objCompany.Company != "") sDriverInfo = sDriverInfo + "\r\n" + objCompany.Company;
                if (objCopyright.Copyright != "") sDriverInfo = sDriverInfo + "\r\n" + objCopyright.Copyright;
                if (objDescription.Description != "") sDriverInfo = sDriverInfo + "\r\n" + objDescription.Description;

                if (SharedResources.Connected)
                {
                    sDriverInfo += "\r\nFirmware version: " + CommandString("GVN", false);
                    sDriverInfo += "\r\nFirmware date: " + CommandString("GVD", false) + " " + CommandString("GVT", false);
                }

                tl.LogMessage("DriverInfo Get", sDriverInfo);
                return sDriverInfo;
            }
        }

        public string DriverVersion
        {
            get
            {
                Version version = System.Reflection.Assembly.GetExecutingAssembly().GetName().Version;
                string driverVersion = String.Format(CultureInfo.InvariantCulture, "{0}.{1}", version.Major, version.Minor);
                tl.LogMessage("DriverVersion Get", driverVersion);
                return driverVersion;
            }
        }

        public short InterfaceVersion
        {
            // set by the driver wizard
            get
            {
                tl.LogMessage("InterfaceVersion Get", "3");
                return Convert.ToInt16("3");
            }
        }

        public string Name
        {
            get
            {
                string name = "ZeGoto";
                tl.LogMessage("Name Get", name);
                return name;
            }
        }

        #endregion

        #region ITelescope Implementation
        public void AbortSlew()
        {
            tl.LogMessage("AbortSlew", "");
            this.CommandBlind("Q", false);                             // Halt the slew
            SharedResources.AbortSlew = true;                                // Raise the abort flag
        }

        public AlignmentModes AlignmentMode
        {
            get
            {
                AlignmentModes ret;
                byte[] ack = { 6 };
                SharedResources.SharedLink.TransmitBinary(ack);
                string buf = SharedResources.SharedLink.ReceiveCounted(1);
                switch (buf)
                {
                    case "A":
                        ret = AlignmentModes.algAltAz;
                        tl.LogMessage("AlignmentMode Get", "algAltAz");
                        break;
                    case "P":
                        ret = AlignmentModes.algGermanPolar;
                        tl.LogMessage("AlignmentMode Get", "algGermanPolar");
                        break;
                    default:
                        tl.LogMessage("AlignmentMode Get", "Unrecognized response : " + buf);
                        throw new ASCOM.PropertyNotImplementedException("AlignmentMode", false);
                }

                return ret;
            }
        }

        public double Altitude
        {
            get
            {
                double altitude = utilities.DMSToDegrees(this.CommandString("GA", false));
                tl.LogMessage("Altitude", "Get - " + utilities.DegreesToDMS(altitude, "°", "'", "''"));
                return altitude;
            }
        }

        public double ApertureArea
        {
            get
            {
                tl.LogMessage("ApertureArea Get", apertureArea.ToString());
                return apertureArea;
            }
        }

        public double ApertureDiameter
        {
            get
            {
                tl.LogMessage("ApertureDiameter Get", apertureDiameter.ToString());
                return apertureDiameter;
            }
        }

        public bool AtHome
        {
            get
            {
                tl.LogMessage("AtHome", "Get - " + false.ToString());
                return false;
            }
        }

        public bool AtPark
        {
            get
            {
                tl.LogMessage("AtPark", "Get - " + SharedResources.isParked.ToString());
                return SharedResources.isParked;
            }
        }

        public IAxisRates AxisRates(TelescopeAxes Axis)
        {
            tl.LogMessage("AxisRates", "Get - " + Axis.ToString());
            return new AxisRates(Axis);
        }

        public double Azimuth
        {
            get
            {
                double azimuth = utilities.DMSToDegrees(this.CommandString("GZ", false));
                tl.LogMessage("Azimuth", "Get - " + utilities.DegreesToDMS(azimuth, "°", "'", "''"));
                return azimuth;
            }
        }

        public double Declination
        {
            get
            {
                double declination = utilities.DMSToDegrees(this.CommandString("GD", false));
                tl.LogMessage("Declination", "Get - " + utilities.DegreesToDMS(declination, "o", "'", "''"));
                return declination;
            }
        }

        public double DeclinationRate
        {
            get
            {
                double declinationRate = 0.0;
                tl.LogMessage("DeclinationRate", "Get - " + declinationRate.ToString());
                return declinationRate;
            }
            set
            {
                tl.LogMessage("DeclinationRate Set", "Not implemented");
                throw new ASCOM.PropertyNotImplementedException("DeclinationRate", true);
            }
        }

        public PierSide DestinationSideOfPier(double RightAscension, double Declination)
        {
            tl.LogMessage("DestinationSideOfPier Get", "Not implemented");
            throw new ASCOM.PropertyNotImplementedException("DestinationSideOfPier", false);
        }

        public bool DoesRefraction
        {
            get
            {
                tl.LogMessage("DoesRefraction Get", "Not implemented");
                throw new ASCOM.PropertyNotImplementedException("DoesRefraction", false);
            }
            set
            {
                tl.LogMessage("DoesRefraction Set", "Not implemented");
                throw new ASCOM.PropertyNotImplementedException("DoesRefraction", true);
            }
        }

        public EquatorialCoordinateType EquatorialSystem
        {
            get
            {
                EquatorialCoordinateType equatorialSystem = EquatorialCoordinateType.equTopocentric;
                tl.LogMessage("DeclinationRate", "Get - " + equatorialSystem.ToString());
                return equatorialSystem;
            }
        }

        public void FindHome()
        {
            tl.LogMessage("FindHome", "Not implemented");
            throw new ASCOM.MethodNotImplementedException("FindHome");
        }

        public double FocalLength
        {
            get
            {
                tl.LogMessage("FocalLength Get", focalLength.ToString());
                return focalLength;
            }
        }

        public double GuideRateDeclination
        {
            get
            {
                string rate = this.CommandString("rG", false);
                double guideRateDeclination = double.Parse(rate) / 10.0;
                guideRateDeclination = 360.0 / (23.0 * 3600.0 + 56.0 * 60.0 + 40) * guideRateDeclination;
                tl.LogMessage("GuideRateDeclination", "Get - :rG# " + rate + " guideRateDeclination=" + guideRateDeclination);
                return guideRateDeclination;
            }
            set
            {
                tl.LogMessage("GuideRateDeclination Set", "Not implemented");
                throw new ASCOM.PropertyNotImplementedException("GuideRateDeclination", true);
            }
        }

        public double GuideRateRightAscension
        {
            get
            {
                string rate = this.CommandString("rG", false);
                double guideRateRightAscension = double.Parse(rate) / 10.0;
                guideRateRightAscension = 360.0 / (23.0 * 3600.0 + 56.0 * 60.0 + 40) * guideRateRightAscension;
                tl.LogMessage("GuideRateRightAscension", "Get - :rG# " + rate + " guideRateRightAscension=" + guideRateRightAscension);
                return guideRateRightAscension;
            }
            set
            {
                tl.LogMessage("GuideRateRightAscension Set", "Not implemented");
                throw new ASCOM.PropertyNotImplementedException("GuideRateRightAscension", true);
            }
        }

        public bool IsPulseGuiding
        {
            get
            {
                tl.LogMessage("IsPulseGuiding", "Get - " + false.ToString());
                return false;
            }
        }

        public void MoveAxis(TelescopeAxes Axis, double Rate)
        {
            double sideral_rate = Rate / (360.0 / 86400.0);
            if (sideral_rate < 0) sideral_rate -= sideral_rate;
            tl.LogMessage("MoveAxis", "Axis=" + Axis.ToString() + " Rate=" + Rate.ToString("0000"));
            CommandBlind("Rs" + Rate.ToString("0000"), false);
            utilities.WaitForMilliseconds(100);

            switch (Axis)
            {
                case TelescopeAxes.axisPrimary:
                    if (Rate >= 0)
                    {
                        // Move east
                        CommandBlind("Me", false);
                    }
                    else
                    {
                        // Move west
                        CommandBlind("Mw", false);
                    }
                    break;
                case TelescopeAxes.axisSecondary:
                    if (Rate >= 0)
                    {
                        // Move north
                        CommandBlind("Mn", false);
                    }
                    else
                    {
                        // Move south
                        CommandBlind("Ms", false);
                    }
                    break;
            }
        }

        public void Park()
        {
            CommandBlind("hP", false);
            tl.LogMessage("Park", "");
            SharedResources.isParked = true;
        }

        public void PulseGuide(GuideDirections Direction, int Duration)
        {
            string d;

            d = "";
            switch (Direction)
            {
                case GuideDirections.guideNorth:
                    d = "n";
                    break;
                case GuideDirections.guideSouth:
                    d = "s";
                    break;
                case GuideDirections.guideEast:
                    d = "e";
                    break;
                case GuideDirections.guideWest:
                    d = "w";
                    break;
            }

            this.CommandBlind("Mg" + d + Duration.ToString("0000"), false);
            tl.LogMessage("PulseGuide", Direction.ToString() + " " + Duration.ToString() + "ms");
        }

        public double RightAscension
        {
            get
            {
                double rightAscension = utilities.HMSToHours(this.CommandString("GR", false));
                tl.LogMessage("RightAscension", "Get - " + utilities.HoursToHMS(rightAscension));
                return rightAscension;
            }
        }

        public double RightAscensionRate
        {
            get
            {
                double rightAscensionRate = 0.0;
                tl.LogMessage("RightAscensionRate", "Get - " + rightAscensionRate.ToString());
                return rightAscensionRate;
            }
            set
            {
                tl.LogMessage("RightAscensionRate Set", "Not implemented");
                throw new ASCOM.PropertyNotImplementedException("RightAscensionRate", true);
            }
        }

        public void SetPark()
        {
            CommandBlind("hP", false);
            tl.LogMessage("SetPark", "Park mount..");
        }

        public PierSide SideOfPier
        {
            get
            {
                PierSide val = PierSide.pierUnknown; ;

                string side = CommandString("pS", false);
                if (side.StartsWith("East"))
                {
                    val = PierSide.pierEast;
                }
                else if (side.StartsWith("West"))
                {
                    val = PierSide.pierWest;
                }
                tl.LogMessage("SideOfPier Get", side + " = " + val.ToString());
                return val;
            }
            set
            {
                tl.LogMessage("SideOfPier Set", "Not implemented");
                throw new ASCOM.PropertyNotImplementedException("SideOfPier", true);
            }
        }

        public double SiderealTime
        {
            get
            {
                // TODO : implementing sideral time command inside ZeGoto
                double siderealTime = (18.697374558 + 24.065709824419081 * (utilities.DateLocalToJulian(DateTime.Now) - 2451545.0)) % 24.0;
                tl.LogMessage("SiderealTime", "Get - " + siderealTime.ToString());
                return siderealTime;
            }
        }

        public double SiteElevation
        {
            get
            {
                if (useGPS == true)
                {
                    CheckConnected("The scope is not connected.");
                    elevation = utilities.DMSToDegrees(CommandString("Gu", false));
                    using (Profile driverProfile = new Profile())
                    {
                        driverProfile.DeviceType = "Telescope";
                        driverProfile.WriteValue(driverID, "Elevation", latitude.ToString(), "");
                    }
                }
                tl.LogMessage("SiteElevation Get", elevation.ToString());
                return elevation;
            }
            set
            {
                elevation = value;
                if (useGPS == false)
                {
                    string e;

                    if (value < 0.0 || value > 9000.0) throw new InvalidOperationException(MSG_PROP_RANGE_ERROR);
                    e = elevation.ToString("0000");
                    if (!this.CommandBool("Se" + e, false))
                    {
                        tl.LogMessage("SiteElevation Set", MSG_TGTSETERR + "Se" + e);
                        throw new InvalidOperationException(MSG_TGTSETERR + "Se" + e);
                    }
                }
                using (Profile driverProfile = new Profile())
                {
                    driverProfile.DeviceType = "Telescope";
                    driverProfile.WriteValue(driverID, "Elevation", latitude.ToString(), "");
                }
                tl.LogMessage("SiteElevation Set", elevation.ToString());
            }
        }

        public double SiteLatitude
        {
            get
            {
                if (useGPS == true)
                {
                    CheckConnected("The scope is not connected.");
                    latitude = utilities.DMSToDegrees(CommandString("Gt", false));
                    using (Profile driverProfile = new Profile())
                    {
                        driverProfile.DeviceType = "Telescope";
                        driverProfile.WriteValue(driverID, "Latitude", latitude.ToString(), "");
                    }
                }
                tl.LogMessage("SiteLatitude Get", latitude.ToString());
                return latitude;
            }
            set
            {
                latitude = value;
                if (useGPS == false)
                {
                    string sexa;

                    if (value < -90.0 || value > 90.0) throw new InvalidOperationException(MSG_PROP_RANGE_ERROR);
                    sexa = utilities.DegreesToDMS(latitude);
                    if (!this.CommandBool("St" + sexa, false))
                    {
                        tl.LogMessage("SiteLatitude Set", MSG_TGTSETERR + "St" + sexa);
                        throw new InvalidOperationException(MSG_TGTSETERR + "St" + sexa);
                    }
                }
                using (Profile driverProfile = new Profile())
                {
                    driverProfile.DeviceType = "Telescope";
                    driverProfile.WriteValue(driverID, "Latitude", latitude.ToString(), "");
                }
                tl.LogMessage("SiteLatitude Set", latitude.ToString());
            }
        }

        public double SiteLongitude
        {
            get
            {
                if (useGPS == true)
                {
                    CheckConnected("The scope is not connected.");
                    longitude = -utilities.DMSToDegrees(CommandString("Gg", false));
                    using (Profile driverProfile = new Profile())
                    {
                        driverProfile.DeviceType = "Telescope";
                        driverProfile.WriteValue(driverID, "Longitude", longitude.ToString(), "");
                    }
                }
                tl.LogMessage("SiteLongitude Get", longitude.ToString());
                return longitude;
            }
            set
            {
                longitude = value;
                if (useGPS == false)
                {
                    string sexa;

                    if (value < -180.0 || value > 180.0) throw new InvalidOperationException(MSG_PROP_RANGE_ERROR);
                    sexa = utilities.DegreesToDMS(longitude);
                    if (!this.CommandBool("Sg" + sexa, false))
                    {
                        tl.LogMessage("SiteLongitude Set", MSG_TGTSETERR + "Sg" + sexa);
                        throw new InvalidOperationException(MSG_TGTSETERR + "Sg" + sexa);
                    }
                }
                using (Profile driverProfile = new Profile())
                {
                    driverProfile.DeviceType = "Telescope";
                    driverProfile.WriteValue(driverID, "Longitude", longitude.ToString(), "");
                }
                tl.LogMessage("SiteLongitude Set", longitude.ToString());
            }
        }

        public short SlewSettleTime
        {
            get
            {
                tl.LogMessage("SlewSettleTime Get", m_iSettleTime.ToString());
                return m_iSettleTime;
            }
            set
            {
                if ((value < 0) || (value > 100))
                {
                    throw new InvalidOperationException(MSG_PROP_RANGE_ERROR);
                }
                m_iSettleTime = value;
                tl.LogMessage("SlewSettleTime Set", value.ToString());
            }
        }

        public void SlewToAltAz(double Azimuth, double Altitude)
        {
            tl.LogMessage("SlewToAltAz", "Not implemented");
            throw new ASCOM.MethodNotImplementedException("SlewToAltAz");
        }

        public void SlewToAltAzAsync(double Azimuth, double Altitude)
        {
            tl.LogMessage("SlewToAltAzAsync", "Not implemented");
            throw new ASCOM.MethodNotImplementedException("SlewToAltAzAsync");
        }

        public void SlewToCoordinates(double RightAscension, double Declination)
        {
            CheckConnected("SlewToCoordinates");
            this.TargetRightAscension = RightAscension;
            this.TargetDeclination = Declination;
            this.SlewToTarget();
            tl.LogMessage("SlewToCoordinates", "RightAscension=" + RightAscension.ToString() + ", Declination=" + Declination.ToString());
        }

        public void SlewToCoordinatesAsync(double RightAscension, double Declination)
        {
            CheckConnected("SlewToCoordinatesAsync");
            this.TargetRightAscension = RightAscension;
            this.TargetDeclination = Declination;
            this.SlewToTargetAsync();
            tl.LogMessage("SlewToCoordinatesAsync", "RightAscension=" + RightAscension.ToString() + ", Declination=" + Declination.ToString());
        }

        public void SlewToTarget()
        {
            CheckConnected("SlewToTarget");
            this.SlewToTargetAsync();   // Start slew
            while (this.Slewing)        // Wait for slew to complete
            {
                utilities.WaitForMilliseconds(500);    // Be responsive to AbortSlew()
                System.Windows.Forms.Application.DoEvents();    // Assure Tithisr runs!
            }
            tl.LogMessage("SlewToTarget", "");
        }

        public void SlewToTargetAsync()
        {
            string resp;
            string msg;
            short i;
            int tmo;

            CheckConnected("SlewToTargetAsync");
            SharedResources.AbortSlew = false;
            m_bStartingSlew = true; // We're TRYING to get the thing to slew...
            //
            // This is where all the work is done for slewing. The other
            // methods eventually call this one. At least one controller
            // (FS2) needs *2* slew commands to work. There are timing gaps
            // between reading the RA & Dec and issuing the MS commands,
            // again for some controllers that choke on closely spaced
            // commands.
            //
            m_dPrevRA = this.RightAscension;
            m_dPrevDec = this.Declination;
            tmo = SharedResources.SharedLink.ReceiveTimeout;
            SharedResources.SharedLink.ReceiveTimeout = 5;
            if (TestSlew(m_dPrevRA, m_dPrevDec, (this.TargetRightAscension), (this.TargetDeclination), m_dRARes * 2.0, m_dDecRes * 2.0))
            {
                for (i = 0; i < 4; i++) // Total of 20 sec...
                {
                    try
                    {
                        utilities.WaitForMilliseconds(200);
                        resp = this.CommandString("MS", false);
                        if (resp[0] != '0')  // Failed to start slew
                        {
                            SharedResources.Slewing = false; // Clear slewing flag
                            m_bStartingSlew = false;
                            switch (resp[0])
                            {
                                case '1':    // Below horizon
                                    msg = MSG_BELOW_HORIZ;
                                    break;
                                case '2':    // Below minimum elevation
                                    msg = MSG_BELOW_MIN_EL;
                                    break;
                                case '3':   // Mount is slewing
                                    SharedResources.Slewing = true;
                                    msg = MSG_SLEWING;
                                    break;
                                case '4':   // Mount is parked
                                    SharedResources.isParked = true;
                                    msg = MSG_PARKED;
                                    break;
                                default:
                                    msg = MSG_SLEW_FAIL;
                                    break;
                            }
                            SharedResources.SharedLink.ReceiveTimeout = tmo;  // Restore tithisout
                            throw new InvalidOperationException(msg); // FAILED
                        }
                        utilities.WaitForMilliseconds(1000);   // Wait for scope to move sothis
                        if (TestSlew(m_dPrevRA, m_dPrevDec, (this.RightAscension), (this.Declination), m_dRARes, m_dDecRes)) break;
                    }
                    catch
                    {
                        utilities.WaitForMilliseconds(500);
                    }
                    //On Error GoTo 0
                }
                SharedResources.SharedLink.ReceiveTimeout = tmo; // Restore timeout
                if (i <= 6) // Success
                {
                    m_lAsyncEndTix = GetTickCount() + ((int)(m_iSettleTime + 4) * 1000);
                    SharedResources.Slewing = true;  // But still say we're slewing
                    m_bStartingSlew = false;
                    return; // Succeeded
                }
                SharedResources.Slewing = false;
                m_bStartingSlew = false;
                throw new InvalidOperationException(MSG_SLEW_FAIL); // Failed!
            }
            SharedResources.Slewing = false; // Too small to slew
            m_bStartingSlew = false;

            tl.LogMessage("SlewToTargetAsync", "");
        }

        public bool Slewing
        {
            get
            {
                bool bMoving;
                double RA, Dec;

                if (!SharedResources.Slewing)        // Short-circuit when not slewing
                {
                    return false;       // No slew now
                }

                if (m_bStartingSlew)    // Starting ... say we're slewing
                {	                    // This is really horrible
                    return true;        // We need a real slewing flag from the scope!
                }

                //
                // This is subtle: If we detect more than resolution of motion, we -know- it's
                // moving, and remember the last position. If not, all we know is that it didn't
                // move more than resolution between requests for the Slewing property. Some
                // client could call this at a high rate, in which case the scope would likely
                // NOT move that far. So we don't update our "last" coordinates if it didn't
                // move far enough. This way, it has to remain within resolution for 3 seconds
                // plus the settling time, or it will reset the end-time and wait another
                // 3 sec plus settling, etc.
                //
                RA = this.RightAscension;
                Dec = this.Declination;
                bMoving = TestSlew(RA, Dec, m_dPrevRA, m_dPrevDec, m_dRARes, m_dDecRes);
                if (bMoving)
                {
                    m_dPrevRA = RA;
                    m_dPrevDec = Dec;
                }

                if (!SharedResources.AbortSlew && bMoving)
                {
                    //
                    // This adds settle time after we see that the scope has
                    // not moved one arcmin.
                    //
                    m_lAsyncEndTix = GetTickCount() + ((int)(m_iSettleTime) * 1000);
                    SharedResources.Slewing = true;
                }
                else
                {
                    //
                    // This implements the trailing settle time
                    // (skip it on a slew abort)
                    //
                    if (SharedResources.AbortSlew || (GetTickCount() > m_lAsyncEndTix))
                    {
                        this.CommandBlind("Q", false);   // Force the scope to stop (FS2 buglet)
                        SharedResources.Slewing = false;
                        SharedResources.AbortSlew = false;
                        System.Media.SystemSounds.Exclamation.Play();
                        //System.Media.SoundPlayer bibip = new System.Media.SoundPlayer(@"c:\Windows\Media\tada.wav");
                        //bibip.Play();
                    }
                    else
                    {
                        SharedResources.Slewing = true;
                    }
                }

                tl.LogMessage("Slewing Get", "");
                return SharedResources.Slewing;

            }
        }

        public void SyncToAltAz(double Azimuth, double Altitude)
        {
            tl.LogMessage("SyncToAltAz", "Not implemented");
            throw new ASCOM.MethodNotImplementedException("SyncToAltAz");
        }

        public void SyncToCoordinates(double RightAscension, double Declination)
        {
            CheckConnected("SyncToCoordinates");
            this.TargetRightAscension = RightAscension;
            this.TargetDeclination = Declination;
            this.SyncToTarget();
            tl.LogMessage("SyncToCoordinates", "RightAscension=" + RightAscension.ToString() + ", Declination=" + Declination.ToString());
        }

        public void SyncToTarget()
        {
            this.CommandString("CM", false);
            tl.LogMessage("SyncToTarget", "");
        }

        public double TargetDeclination
        {
            get
            {
                if (!m_bTargetDecValid)
                {
                    tl.LogMessage("TargetDeclination Get", "Property TargetDeclination " + MSG_PROPNOTSET);
                    throw new InvalidOperationException("Property TargetDeclination " + MSG_PROPNOTSET);
                }

                tl.LogMessage("TargetDeclination Get", m_dTargetDec.ToString());
                return m_dTargetDec;
            }
            set
            {
                string sexa;

                CheckConnected("TargetRightAscension");
                if (value < -90.0 || value > 90.0) throw new InvalidOperationException(MSG_PROP_RANGE_ERROR);
                if (!SharedResources.LongFormat)
                {
                    sexa = utilities.DegreesToDM(value, SharedResources.DecDelimDeg, SharedResources.DecDelimMin, 1);    // Short format, 1 decimal digit
                    tl.LogMessage("TargetDeclination Set short ", sexa);
                }
                else
                {
                    sexa = utilities.DegreesToDMS(value, SharedResources.DecDelimDeg, SharedResources.DecDelimMin, SharedResources.DecDelimSec, 0);  // Long format, whole seconds
                    tl.LogMessage("TargetDeclination Set long ", sexa);
                }

                if (sexa.Substring(0, 1) != "-") sexa = "+" + sexa;  // Both need leading '+'
                if (!this.CommandBool("Sd" + sexa, false))
                {
                    tl.LogMessage("TargetDeclination Set", MSG_TGTSETERR + "TargetDeclination = " + sexa);
                    throw new InvalidOperationException(MSG_TGTSETERR + "TargetDeclination = " + sexa);
                }

                m_dTargetDec = value;   // Set shadow variables
                m_bTargetDecValid = true;

                tl.LogMessage("TargetDeclination Set", value.ToString());
            }
        }

        public double TargetRightAscension
        {
            get
            {
                if (!m_bTargetRAValid)
                {
                    tl.LogMessage("TargetRightAscension Get", "Property TargetRightAscension " + MSG_PROPNOTSET);
                    throw new InvalidOperationException("Property TargetRightAscension " + MSG_PROPNOTSET);
                }
                tl.LogMessage("TargetRightAscension Get", m_dTargetRA.ToString());
                return m_dTargetRA;
            }
            set
            {
                string sexa;

                CheckConnected("TargetRightAscension");
                if (value < 0.0 || value >= 24.0) throw new InvalidOperationException(MSG_PROP_RANGE_ERROR);
                if (!SharedResources.LongFormat)
                {
                    sexa = utilities.HoursToHM(value, SharedResources.RADelimHrs, SharedResources.RADelimMin, 1);    // Short format, 1 decimal digit
                }
                else
                {
                    sexa = utilities.HoursToHMS(value, SharedResources.RADelimHrs, SharedResources.RADelimMin, SharedResources.RADelimSec, 0);   // Long format, whole seconds
                }

                if (!this.CommandBool("Sr" + sexa, false))
                {
                    tl.LogMessage("TargetRightAscension Set", MSG_TGTSETERR + "TargetRightAscension = " + sexa);
                    throw new InvalidOperationException(MSG_TGTSETERR + "TargetRightAscension = " + sexa);
                }

                m_dTargetRA = value;    // Set shadow variables
                m_bTargetRAValid = true;

                tl.LogMessage("TargetRightAscension Set", value.ToString());
                tl.LogMessage("TargetRightAscension Set", sexa);
            }
        }

        public bool Tracking
        {
            get
            {
                bool tracking = true;
                tl.LogMessage("Tracking", "Get - " + tracking.ToString());
                return tracking;
            }
            set
            {
                tl.LogMessage("Tracking Set", "Not implemented");
                //throw new ASCOM.PropertyNotImplementedException("Tracking", true);
            }
        }

        public DriveRates TrackingRate
        {
            get
            {
                DriveRates r = DriveRates.driveSidereal;
                tl.LogMessage("TrackingRate Get", r.ToString());
                return r;
            }
            set
            {
                tl.LogMessage("TrackingRate Set", "Not implemented");
                throw new ASCOM.PropertyNotImplementedException("TrackingRate", true);
            }
        }

        public ITrackingRates TrackingRates
        {
            get
            {
                ITrackingRates trackingRates = new TrackingRates();
                tl.LogMessage("TrackingRates", "Get - ");
                foreach (DriveRates driveRate in trackingRates)
                {
                    tl.LogMessage("TrackingRates", "Get - " + driveRate.ToString());
                }
                return trackingRates;
            }
        }

        public DateTime UTCDate
        {
            get
            {
                DateTime utcDate = DateTime.UtcNow;
                tl.LogMessage("TrackingRates", "Get - " + String.Format("MM/dd/yy HH:mm:ss", utcDate));
                return utcDate;
            }
            set
            {
                tl.LogMessage("UTCDate Set", "Not implemented");
                throw new ASCOM.PropertyNotImplementedException("UTCDate", true);
            }
        }

        public void Unpark()
        {
            CommandBlind("hW", false);
            tl.LogMessage("Unpark", "");
            SharedResources.isParked = false;
        }

        #endregion

        #region Private properties and methods
        // here are some useful properties and methods that can be used as required
        // to help with driver development

        #region ASCOM Registration

        //// Register or unregister driver for ASCOM. This is harmless if already
        //// registered or unregistered. 
        ////
        ///// <summary>
        ///// Register or unregister the driver with the ASCOM Platform.
        ///// This is harmless if the driver is already registered/unregistered.
        ///// </summary>
        ///// <param name="bRegister">If <c>true</c>, registers the driver, otherwise unregisters it.</param>
        //private static void RegUnregASCOM(bool bRegister)
        //{
        //    using (var P = new ASCOM.Utilities.Profile())
        //    {
        //        P.DeviceType = "Telescope";
        //        if (bRegister)
        //        {
        //            P.Register(driverID, driverDescription);
        //        }
        //        else
        //        {
        //            P.Unregister(driverID);
        //        }
        //    }
        //}

        ///// <summary>
        ///// This function registers the driver with the ASCOM Chooser and
        ///// is called automatically whenever this class is registered for COM Interop.
        ///// </summary>
        ///// <param name="t">Type of the class being registered, not used.</param>
        ///// <remarks>
        ///// This method typically runs in two distinct situations:
        ///// <list type="numbered">
        ///// <item>
        ///// In Visual Studio, when the project is successfully built.
        ///// For this to work correctly, the option <c>Register for COM Interop</c>
        ///// must be enabled in the project settings.
        ///// </item>
        ///// <item>During setup, when the installer registers the assembly for COM Interop.</item>
        ///// </list>
        ///// This technique should mean that it is never necessary to manually register a driver with ASCOM.
        ///// </remarks>
        //[ComRegisterFunction]
        //public static void RegisterASCOM(Type t)
        //{
        //    RegUnregASCOM(true);
        //}

        ///// <summary>
        ///// This function unregisters the driver from the ASCOM Chooser and
        ///// is called automatically whenever this class is unregistered from COM Interop.
        ///// </summary>
        ///// <param name="t">Type of the class being registered, not used.</param>
        ///// <remarks>
        ///// This method typically runs in two distinct situations:
        ///// <list type="numbered">
        ///// <item>
        ///// In Visual Studio, when the project is cleaned or prior to rebuilding.
        ///// For this to work correctly, the option <c>Register for COM Interop</c>
        ///// must be enabled in the project settings.
        ///// </item>
        ///// <item>During uninstall, when the installer unregisters the assembly from COM Interop.</item>
        ///// </list>
        ///// This technique should mean that it is never necessary to manually unregister a driver from ASCOM.
        ///// </remarks>
        //[ComUnregisterFunction]
        //public static void UnregisterASCOM(Type t)
        //{
        //    RegUnregASCOM(false);
        //}

        #endregion


        /// <summary>
        /// Use this function to throw an exception if we aren't connected to the hardware
        /// </summary>
        /// <param name="message"></param>
        private void CheckConnected(string message)
        {
            if (!SharedResources.Connected)
            {
                throw new ASCOM.NotConnectedException(message);
            }
        }

        /// <summary>
        /// Read the device configuration from the ASCOM Profile store
        /// </summary>
        internal void ReadProfile()
        {
            using (Profile driverProfile = new Profile())
            {
                driverProfile.DeviceType = "Telescope";

                apertureDiameter = Convert.ToDouble(driverProfile.GetValue(driverID, "ApertureDiameter", string.Empty, "0"));
                focalLength = Convert.ToDouble(driverProfile.GetValue(driverID, "FocalLength", string.Empty, "0"));
                apertureArea = 3.1415926535897932384626433832795 * Math.Pow(apertureDiameter / 2, 2);

                useGPS = Convert.ToBoolean(driverProfile.GetValue(driverID, "UseGPS", string.Empty, true.ToString()));
                latitude = Convert.ToDouble(driverProfile.GetValue(driverID, "Latitude", string.Empty, "0"));
                longitude = Convert.ToDouble(driverProfile.GetValue(driverID, "Longitude", string.Empty, "0"));
                elevation = Convert.ToDouble(driverProfile.GetValue(driverID, "Elevation", string.Empty, "0"));

                traceState = Convert.ToBoolean(driverProfile.GetValue(driverID, traceStateProfileName, string.Empty, traceStateDefault));
            }
        }

        /// <summary>
        /// Write the device configuration to the  ASCOM  Profile store
        /// </summary>
        internal void WriteProfile()
        {
            using (Profile driverProfile = new Profile())
            {
                driverProfile.DeviceType = "Telescope";

                driverProfile.WriteValue(driverID, "ApertureDiameter", apertureDiameter.ToString());
                driverProfile.WriteValue(driverID, "FocalLength", focalLength.ToString());

                driverProfile.WriteValue(driverID, "UseGPS", useGPS.ToString());
                driverProfile.WriteValue(driverID, "Latitude", latitude.ToString());
                driverProfile.WriteValue(driverID, "Longitude", longitude.ToString());
                driverProfile.WriteValue(driverID, "Elevation", elevation.ToString());

                driverProfile.WriteValue(driverID, traceStateProfileName, traceState.ToString());
            }

            Configuration driverConfig = ConfigurationManager.OpenExeConfiguration(ConfigurationUserLevel.None);
            driverConfig.AppSettings.Settings.Clear();

            driverConfig.AppSettings.Settings.Add("ApertureDiameter", apertureDiameter.ToString());
            driverConfig.AppSettings.Settings.Add("FocalLength", focalLength.ToString());

            driverConfig.AppSettings.Settings.Add("UseGPS", useGPS.ToString());
            driverConfig.AppSettings.Settings.Add("Latitude", latitude.ToString());
            driverConfig.AppSettings.Settings.Add("Longitude", longitude.ToString());
            driverConfig.AppSettings.Settings.Add("Elevation", elevation.ToString());

            driverConfig.AppSettings.Settings.Add(traceStateProfileName, traceState.ToString());
            driverConfig.Save();
        }

        /// <summary>
        /// Set scope "long" format on or off
        /// 
        /// The scope has only a toggle for the format, so we need to
        /// first test the format and then conditionally issue the
        /// toggle to set the desired format.
        ///
        /// Returns the format after the function is called. Some
        /// LX200 emulators don't support long format.
        /// </summary>
        /// <param name="b"></param>
        /// <returns></returns>
        private bool SetLongFormat(bool b)
        {
            string buf;

            buf = this.CommandString("GR", false); // Get RA
            //
            // Scope has some sort of race condition with "U", so
            // we just wait a half sec. here and then do it.
            //
            utilities.WaitForMilliseconds(500);
            if ((b && (buf.Length <= 7)) || (!b && (buf.Length > 7)))
                this.CommandBlind("U", false);      // change setting

            buf = this.CommandString("GR", false);  // Check final result
            if (buf.Length <= 7)
                return false;   // Now/still in short format
            return true;        // Now in long format
        }

        /// <summary>
        /// Test to see if the differences in RA and Dec exceed the
        /// corresponding resolution.
        /// </summary>
        /// <param name="ra1"></param>
        /// <param name="dec1"></param>
        /// <param name="ra2"></param>
        /// <param name="dec2"></param>
        /// <param name="raRes"></param>
        /// <param name="decRes"></param>
        /// <returns></returns>
        private bool TestSlew(double ra1, double dec1, double ra2, double dec2, double raRes, double decRes)
        {
            double diff;

            diff = ra2 - ra1;
            if (diff > 12.0) diff = 24.0 - diff;
            if (diff < -12.0) diff = diff + 24.0;
            if ((System.Math.Abs(diff) * 15.0) > raRes)
                return true;    // Difference in RA is enough

            diff = System.Math.Abs(dec2 - dec1);
            if (diff > decRes)
                return true;    // Difference in Dec is enough

            return false;
        }

        /// <summary>
        /// Trim leading/trailing spaces from delimiters. If the result is
        /// "" then the delimiter was a single space, add that back in.
        /// </summary>
        /// <param name="s">string to trim</param>
        /// <returns>string</returns>
        private string DelimTrim(string s)
        {
            string d = s.Trim();
            if (d == "") d = " ";
            return d;
        }

        #endregion

    }
}
