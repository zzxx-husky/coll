#pragma once

#include <vector>

#include "zaf/zaf.hpp"

namespace coll {
class ActorGroup {
public:
  static ActorGroup create(size_t num_actors) {
    ActorGroup group;
    group.actors.resize(num_actors);
    for (auto& a : actors) {
      a = system.spawn([&](auto& self) {
      });
    }
    return group;
  }

private:
  static zaf::ActorSystem system;

  ActorGroup() = default;

  std::vector<zaf::Actor> actors;
};
} // namespace coll
