using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace DungeonEditorZelda
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }

        public void Decompress(int apos)
        {
            byte b = 0;
            int pos = 0;
            /*bits
76543210
CCCLLLLL

CCC:   Command bits
LLLLL: Length*/
            /*000    "Direct Copy"
       Followed by (L+1) bytes of data
001    "Byte Fill"
       Followed by one byte to be repeated (L+1) times
010    "Word Fill"
       Followed by two bytes. Output first byte, then second, then first,
       then second, etc. until (L+1) bytes has been outputted
011    "Increasing Fill"
       Followed by one byte to be repeated (L+1) times, but the byte is
       increased by 1 after each write
100    "Repeat"
       Followed by two bytes (ABCD byte order) containing address (in the
       output buffer) to copy (L+1) bytes from
101    (Unused command)
110    (Unused command)
111    "Long length"
       This command has got a two-byte header:
       111CCCLL LLLLLLLL
       CCC:        Real command
       LLLLLLLLLL: Length*/



        }
    }
}
