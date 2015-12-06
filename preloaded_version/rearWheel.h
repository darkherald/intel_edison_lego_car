#include <mutex>
#include <thread>
#include <unistd.h>
#include <mraa/gpio.h>

class RearWheel {
public:
	RearWheel() {
		count = 0;
		isRun = false;
	}

	void clear() {
		std::lock_guard<std::mutex> g(this->m);
		this->isRun = false;
		this->count = 0;
	}

	float getCount() {
		std::lock_guard<std::mutex> g(this->m);
		return this->count;
	}

	void update() {
		std::cout << "Start to update...\n";
		this->m.lock();
		this->isRun = true;
		this->m.unlock();
		mraa_gpio_context outA, outB;
		outA = mraa_gpio_init(2);
		outB = mraa_gpio_init(3);
		mraa_gpio_dir(outA, MRAA_GPIO_IN);
		mraa_gpio_dir(outB, MRAA_GPIO_IN);
		int a = 0, b = 0;
		while (!a)
			a = mraa_gpio_read(outA);
		while (this->isRunning()) {
			while (a) {
				a = mraa_gpio_read(outA);
				b = mraa_gpio_read(outB);
			}
			increment();
			while (!a) {
				a = mraa_gpio_read(outA);
				b = mraa_gpio_read(outB);
			}
			increment();
			usleep(10);
		}
		std::cout << "End updating.\n";
		this->m.lock();
		this->count = 0;
		this->m.unlock();
	}

	std::thread threading() {
		std::thread t1(&RearWheel::update, this);
		return t1;
	}

private:
	std::mutex m;
	float count;
	bool isRun;

	void increment() {
		std::lock_guard<std::mutex> g(this->m);
		this->count += 0.5;
	}

	bool isRunning() {
		std::lock_guard<std::mutex> g(this->m);
		return this->isRun;
	}
};
