#include "main.h"

int main(int argc, char** argv)
{
	QApplication a(argc, argv);
	enqu::main_window q;
	q.show();
	return a.exec();
}
