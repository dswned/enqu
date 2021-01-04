#include <QApplication>
#include <QMainWindow>

namespace enqu {

class main_window : public QMainWindow
{
	Q_OBJECT
public:
	main_window();
	~main_window();
protected:
	bool eventFilter(QObject*, QEvent*) override;
};

}
