#include "coll/coll.hpp"
#include "coll/parallel_coll.hpp"

#include "webdriverxx/webdriverxx.h"

int main(int argc, char** argv) {
  int npages = 1;
  std::string key = "zzxx coll github";

  coll::iterate(argv + 1, argv + argc)
    | coll::window(2)
    | coll::foreach([&](auto&& w) {
        if (w[0] == std::string("npages")) {
          npages = std::stoi(w[1]);
          npages = npages < 1 ? 1 : npages;
        }
        if (w[0] == std::string("key")) {
          key = coll::iterate(w[2])
            | coll::map(anony_cc(_ <= ' ' ? '+' : _))
            | coll::to<std::string>();
        }
      });

  auto browser = []() {
    try {
      webdriverxx::ChromeOptions opts;
      opts.SetArgs({"--headless"});
      return webdriverxx::Start(webdriverxx::Chrome().SetChromeOptions(opts), "http://127.0.0.1:4444/wd/hub/");
    } catch (...) {
      LOG(ERROR) << "Failed to start webdriver. Did you start standalone selenium server with a proper browser driver?" << std::endl
        << "  Note that for this program, you will need to do the following steps:" << std::endl
        << "  1. Install Chromium or Google Chrome, i.e., a browser" << std::endl
        << "  2. Install ChromeDriver that matches the version of the browser. You may need to tell ChromeDriver where the browser is installed." << std::endl
        << "     (from https://chromedriver.chromium.org/downloads)" << std::endl
        << "  3. Install Selenium server. You may need to tell Selenium server where ChromeDriver is installed." << std::endl
        << "     (from https://selenium-release.storage.googleapis.com)" << std::endl
        << "  4. Start Selenium server standalone." << std::endl
        << "     (java -jar selenium-server-xxx.jar standalone)" << std::endl
        << "  5. Then this client library will talk to Selenium server who operates the browser by the driver and sends the results back" << std::endl
        << std::endl;
      throw;
    }
  }();

  // sequentially iterate the search results for specific key within specific number of pages
  coll::range(npages) | coll::foreach([&](int p) {
    if (p == 0) {
      browser.Navigate("https://www.google.com/");
      auto elem = browser
        .FindElement(webdriverxx::ByName("q"))
        .SendKeys(key)
        .Submit();
    }
  });
}
