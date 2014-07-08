using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Threading;

namespace DarkFriendsInjector
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        Thread backgroundThread;
        public MainWindow()
        {
            InitializeComponent();
            backgroundThread = new Thread(new ThreadStart(threadEntry));
            backgroundThread.Start();
        }

        bool execute = false;
        bool res = false;
        void threadEntry()
        {
            do
            {
                Thread.Sleep(500);
                try
                {
                    if(execute)
                        res = HookInject.Hook();
                }
                catch (Exception ex)
                {
                    Dispatcher.Invoke(delegate()
                    {
                        statusText.Text = "Error: " + ex.Message;
                    }
                    );
                }
                if (res)
                {
                    Dispatcher.Invoke(delegate()
                   {
                       enableButton.IsEnabled = false;
                       disableButton.IsEnabled = false;
                       statusText.Text = "Injected successfully";
                   });
                    execute = false;
                }
            }
            while (res == false);
        }

        private void enableButton_Click(object sender, RoutedEventArgs e)
        {
            disableButton.IsEnabled = true;
            enableButton.IsEnabled = false;
            statusText.Text = "Searching for Dark Souls II instance";
            execute = true;
        }

        private void disableButton_Click(object sender, RoutedEventArgs e)
        {
            disableButton.IsEnabled = false;
            enableButton.IsEnabled = true;
            execute = false;
            statusText.Text = "Idle";
        }

        protected override void OnClosed(EventArgs e)
        {
            base.OnClosed(e);
            backgroundThread.Abort();
            Application.Current.Shutdown();
        }
    }
}
