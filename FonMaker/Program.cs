using System;
using System.Windows;
using System.Windows.Forms;
using System.Drawing;

class Program
{
    [STAThread]
    public static void Main(string[] args)
    {
        Application.EnableVisualStyles();
        Application.SetCompatibleTextRenderingDefault(false);
        Application.Run(new MainForm());
    }
}

class MainForm : Form
{
    private PictureBox _pictureBoxGrid;
    private const int GRID_COLS = 16;
    private const int GRID_ROWS = 8;
    private bool[,] _gridStatus;
    private const int CELL_SIZE = 30;

    public MainForm()
    {
        _pictureBoxGrid = new PictureBox { Dock = DockStyle.Fill, BorderStyle = BorderStyle.FixedSingle };
       
        this.Controls.Add(_pictureBoxGrid);

        _gridStatus = new bool[GRID_COLS, GRID_ROWS];

        for(int i=0; i<GRID_COLS; i++)
        {
            for(int j=0; j<GRID_ROWS; j++)
            {
                _gridStatus[i, j] = false;
            }
        }

        this.ClientSize = new Size(GRID_ROWS * CELL_SIZE, GRID_COLS * CELL_SIZE);
        this.FormBorderStyle = FormBorderStyle.FixedSingle;

        _pictureBoxGrid.Paint += _pictureBoxGrid_Paint;
        _pictureBoxGrid.MouseClick += _pictureBoxGrid_MouseClick;

        this.KeyPress += MainForm_KeyPress;
    }

    private void MainForm_KeyPress(object? sender, KeyPressEventArgs e)
    {
        if(e.KeyChar.ToString().ToUpper()[0] == 'S')
        {
            byte[] raw = ConvertToBitmapFont();
            string result = "";
            for(int i=0; i< raw.Length; i++)
            {
                result += "0b" + Convert.ToString(raw[i], 2).PadLeft(GRID_ROWS, '0');
                if(i < raw.Length - 1)
                {
                    result += ',';
                }
            }

            Clipboard.SetText(result);
            return;
        }

        if (e.KeyChar.ToString().ToUpper()[0] == 'C')
        {
            if (MessageBox.Show("Erase all bitmaps, continue?", "Caution", MessageBoxButtons.OKCancel, MessageBoxIcon.Warning) == DialogResult.OK)
            {
                for (int i = 0; i < GRID_COLS; i++)
                {
                    for (int j = 0; j < GRID_ROWS; j++)
                    {
                        _gridStatus[i, j] = false;
                    }
                }
            }

            _pictureBoxGrid.Invalidate();

            return;
        }
    }

    private byte[] ConvertToBitmapFont()
    {
        byte[] result = new byte[GRID_COLS];
        for (int i = 0; i < GRID_COLS; i++)
        {
            result[i] = 0;

            for (int j = 0; j < GRID_ROWS; j++)
            {
                result[i] |= (byte)((_gridStatus[i, j] ? (1 << (GRID_ROWS - j - 1)) : 0));
            }
        }

        return result;
    }

    private void _pictureBoxGrid_MouseClick(object? sender, MouseEventArgs e)
    {
        int col = e.X / CELL_SIZE;
        int row = e.Y / CELL_SIZE;

        if (row >= 0 && row < GRID_COLS && col >= 0 && col < GRID_ROWS)
        {
            _gridStatus[row, col] = !_gridStatus[row, col];
            _pictureBoxGrid.Invalidate(new Rectangle(col * CELL_SIZE, row * CELL_SIZE, CELL_SIZE, CELL_SIZE));
        }
    }

    private void _pictureBoxGrid_Paint(object? sender, PaintEventArgs e)
    {
        Graphics g = e.Graphics;
        g.Clear(Color.White);

        for (int i = 0; i < GRID_COLS; i++)
        {
            for (int j = 0; j < GRID_ROWS; j++)
            {
                Brush cellBrush = _gridStatus[i, j] ? Brushes.Black : Brushes.White;
                g.FillRectangle(cellBrush, j * CELL_SIZE, i * CELL_SIZE, CELL_SIZE, CELL_SIZE);
                g.DrawRectangle(Pens.DarkGray, j * CELL_SIZE, i * CELL_SIZE, CELL_SIZE, CELL_SIZE);
            }
        }
    }
}