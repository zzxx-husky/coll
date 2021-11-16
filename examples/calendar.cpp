#include <ctime>

#include <iomanip>
#include <sstream>

#include "coll/coll.hpp"

struct Date {
  int month;
  int day_in_month;
  int week_in_month;
};

void make_calendar(int year, int months_per_row) {
  const static int nday_in_months[12] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
  };

  const static std::string month_names[12] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };

  auto is_leap_year = [](int year) {
    return year % 400 == 0 || (year % 4 == 0 && year % 100 != 0);
  };

  int day_in_week_for_day_1 = [&]() {
    // 2000 Jan 1 is Saturday
    auto nday = *((year < 2000 ? coll::range(year, 2000) : coll::range(2000, year))
      | coll::map(anonyr_cc((365 + is_leap_year(_)) % 7))
      | coll::sum()) % 7;
    return year < 2000 ? 6 - nday : (6 + nday) % 7;
  }();

  coll::generate([&, month = 0, day_in_month = 0, week_in_month = 0,
                  day_in_week = day_in_week_for_day_1, nday_in_month = nday_in_months[0]]() mutable {
        Date res{month, day_in_month, week_in_month};
        if (++day_in_week == 7) {
          day_in_week = 0;
          ++week_in_month;
        }
        if (++day_in_month == nday_in_month) {
          day_in_month = 0;
          week_in_month = 0;
          ++month;
          nday_in_month = nday_in_months[month] + (month == 1 && is_leap_year(year));
        }
        return res;
      }).times(365 + is_leap_year(year))
    // ((month, week_in_month), week oss)
    | coll::groupby(anony_ac(std::make_pair(_.month, _.week_in_month)))
        .adjacent()
        .aggregate([](auto) { return std::ostringstream(); }, [](auto& out, auto&& d) {
          out << std::setw(3) << (d.day_in_month + 1);
        })
    // (month, week str)
    | coll::map([](auto&& mw2oss) {
        auto week = mw2oss.second.str();
        return std::make_pair(
          mw2oss.first.first, 
          week.length() == 7 * 3
            ? std::move(week)
            : [&]() {
                std::ostringstream oss;
                oss << (mw2oss.first.second == 0 ? std::right : std::left) << std::setw(7 * 3) << week;
                return oss.str();
              }()
        );
      })
    // (month, [week str])
    | coll::groupby(anony_rc(_.first))
        .valueby(anony_rc(_.second))
        .adjacent()
        .to_vector()
    // [(month, [week str]) * months_per_row]
    | coll::window(months_per_row)
    | coll::println().format([&](auto& out, auto& w) mutable {
        // One way to print the calendar with less coll::print
        // coll::iterate(w) | coll::map(anony_rc(std::string(7 * 3 / 2 - 1, ' ') + month_names[_.first] + std::string(7 * 3 / 2 - 1, ' ')))
        //   | coll::concat( coll::generate(anony_c(" Su Mo Tu We Th Fr Sa")).times(w.size()) )
        //   | coll::concat( coll::range(6) | coll::flatmap([&](int week) {
        //       return coll::iterate(w) | coll::map(anonyc_rc(week < _.second.size() ? _.second[week] : std::string(7 * 3, ' ')));
        //     }) )
        //   | coll::print("", "", "").to(out).format([&, cnt = 0](auto& out, auto&& str) mutable {
        //       out << str << (++cnt % w.size() ? "     " : "\n");
        //     });

        // Another way to print the calendar with less std::string
        coll::iterate(w) | coll::map(anonyr_ar(month_names[_.first]))
          | coll::print("", "     ", "\n").to(out).format([](auto& out, auto& month_name) {
              out << std::setw(7 * 3 / 2 + 2) << month_name << std::setw(7 * 3 / 2 - 1) << "";
            });
        coll::generate(anony_c(" Su Mo Tu We Th Fr Sa")).times(w.size())
          | coll::print("", "     ", "\n").to(out);
        coll::range(6)
          | coll::inspect([&](int week) {
              coll::iterate(w) | coll::print("", "     ", "\n").to(out).format([&](auto& out, auto& i) {
                out << std::setw(7 * 3) << (week < i.second.size() ? i.second[week].data() : "");
              });
            })
          | coll::act();
      });
}

int main(int argc, char** argv) {
  int year = []() {
    auto t = time(nullptr);
    auto tm = localtime(&t);
    return tm->tm_year + 1900;
  }();
  int width = 3;
  coll::range(1, argc)
    | coll::map(anonyc_cc(argv[_]))
    | coll::window(2)
    | coll::foreach([&](auto& w) {
        if (strcmp(w[0], "year") == 0) {
          year = atoi(w[1]);
        } else if (strcmp(w[0], "width") == 0) {
          width = atoi(w[1]);
        } else {
          std::cout << "Unknown arg: " << w[0] << std::endl;
        }
      });
  make_calendar(year, width);
}
