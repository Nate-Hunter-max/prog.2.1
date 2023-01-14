//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include <queue>
#include <iostream>

using namespace Prog_2_1;

using namespace Platform;
using namespace std;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Concurrency;
using namespace std;


class Request {
private:
	size_t index;
	bool isWashed;
	bool isRefueled;
public:
	Request(size_t index, bool isWashed = false, bool isRefueled = false)
	{
		this->index = index;
		this->isWashed = isWashed;
		this->isRefueled = isRefueled;
	}
	size_t getIndex() { return index; };
	void wash() { isWashed = true; };
	void refuel() { isRefueled = true; };
	bool washed() { return isWashed; };
	bool refueled() { return isRefueled; };
};

class ModelQueue {
private:
	queue<Request> q;
	size_t full;
	double QueueAverage;
	size_t MaxLen;
public:
	ModelQueue(size_t fullLen) { full = fullLen; }
	void addReq(Request& r) { q.push(r); if (len() > MaxLen)MaxLen = len(); }
	Request& removeFirst() { Request& r = q.front(); q.pop(); return r; }
	size_t len() { return q.size(); }
	size_t maxLen() { return MaxLen; }
	bool isFull() { return q.size() == full; }
	bool isEmpty() { return q.empty(); }
	double getAverage(size_t tick) {
		QueueAverage = (QueueAverage * (tick - 1) + len()) / tick;
		return QueueAverage;
	};
};

class Generator {
private:
	size_t tick;
	int rpm;
	size_t reqIndex;
	int requestCount;
	void generate() {
		if (tick % (60 / rpm) == 0) {
			requestCount++;
			reqIndex++;
		}
	}
public:
	Request removeReq() {
		requestCount--;
		Request current(reqIndex);
		return current;
	}
	Generator(int requestsPerMinute)
	{
		tick = 0;
		requestCount = 0;
		reqIndex = 0;
		rpm = requestsPerMinute;
	}
	void nextTick(size_t ticks = 1) { tick += ticks; generate(); }
	size_t gerReqCount() { return requestCount; };
};

class RequestHandler {
private:
	short type;
	float rpm;
	bool Ready;
	size_t tick;
public:
	RequestHandler(short type, float rpm) { this->type = type; this->rpm = rpm; Ready = true; tick = 0;}
	void Handle(Request& r) {
		if (Ready) {
			switch (type)
			{
			case 1:
				r.wash();
				break;
			case 2:
				r.refuel();
				break;
			}
		}
		tick = 0;
	}
	void nextTick(ModelQueue q, size_t ticks = 1) { if(!q.isEmpty())tick+=ticks; Ready = tick >= 60 / rpm; }
	bool isReady() { return Ready; }
};
class StatisticsManager {
private:
	size_t ReqCounter;
	size_t GenCounter;
	size_t WashedReqCounter;
	size_t RefueledReqCounter;
	size_t RefueledAndWashedReqCounter;
public:
	StatisticsManager() { ReqCounter = WashedReqCounter = RefueledReqCounter = GenCounter = RefueledAndWashedReqCounter= 0; }
	void processReq(Request& r) {
		ReqCounter++;
		WashedReqCounter += r.washed();
		RefueledReqCounter += r.refueled();
		RefueledAndWashedReqCounter += (r.washed() && r.refueled());
	}
	double washedPercent() { return (double)WashedReqCounter / ReqCounter * 100; };
	double refueledPercent() { return (double)RefueledReqCounter / ReqCounter * 100; };
	double refWashPercent() { return (double)RefueledAndWashedReqCounter / ReqCounter * 100; };
	void incGenCounter() { GenCounter++; };
	size_t getGenCount() { return GenCounter; };
	size_t getReqCount() { return ReqCounter; };
	size_t getwashedCount() { return WashedReqCounter; };
};
class Valve {
private:
	bool state;
public:
	Valve(bool initState) { state = initState; }
	void setOpen(bool state) { this->state = state; }
	bool isOpen() { return state; }
};
MainPage::MainPage()
{

	InitializeComponent();

}


void Prog_2_1::MainPage::BtStartClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Generator gen(4);
	ModelQueue q1(4), q2(1);
	RequestHandler d1(1, 0.5 * 4), d2(2, 2);
	Valve k1(true), k2(false), k3(true), k4(false);
	StatisticsManager stat;
	Request& req = Request(0);
	TbOutMain->Text = "";
	for (size_t i = 1; i <= stoi(TbDays->Text->Data()) * 60 * 60; i += 1)
	{
		gen.nextTick(1); d1.nextTick(q1,1); d2.nextTick(q2,1);
		if (gen.gerReqCount() > 0) {
			req = gen.removeReq();
			stat.incGenCounter();
			if (k1.isOpen()) {
				q1.addReq(req);
				if (q1.isFull()) { k1.setOpen(false); k2.setOpen(true); };
			}
			else if (k2.isOpen()) {
				stat.processReq(req);
			};
		}
		if (d1.isReady() && !q1.isEmpty()) {
			req = q1.removeFirst();
			d1.Handle(req); k1.setOpen(true); k2.setOpen(false);
			if (k3.isOpen()) {
				q2.addReq(req);
				k3.setOpen(!q2.isFull());
				k4.setOpen(!k3.isOpen());
			}
			else if (k4.isOpen()) {
				stat.processReq(req);
			}
		}
		if (d2.isReady() && !q2.isEmpty()) {
			req = q2.removeFirst();
			d2.Handle(req); k3.setOpen(true); k4.setOpen(false);
			stat.processReq(req);
		}
		TbQ1Len->Text = q1.getAverage(i).ToString();
		TbQ2Len->Text = q2.getAverage(i).ToString();
		TbTime->Text = (i / 60).ToString() + " мин.";
		if (i % (stoi(TbStep->Text->Data()) * 60) == 0) {
			TbOutMain->Text += "Прошло " + i/60 + " мин. " + "Заявок создано: " + stat.getGenCount() + ", обработано: "
				+ stat.getReqCount() + ", длины очередей Q1,Q2: " + q1.len() + ", " + q2.len()+"\n  "+
				"Машин помыто и заправлено: "+stat.getwashedCount()+"\n\n";
		}

	}
	TbGen->Text = stat.getGenCount().ToString();
	TbReq->Text = stat.getReqCount().ToString();
	TbWashed->Text = stat.washedPercent() + "%";
	TbRefuiled->Text = stat.refueledPercent() + "%";
	TbWashedAndRef->Text = stat.refWashPercent() + "%";
	TbQ1Max->Text = q1.maxLen().ToString();
	TbQ2Max->Text = q2.maxLen().ToString();
}
