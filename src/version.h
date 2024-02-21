#ifndef VERSION_H_C757E53C
#define VERSION_H_C757E53C

#include <iostream>
#include <regex>

struct Version {
  int major = 0;
  int minor = 0;
  int revision = 0;

  constexpr Version(int major_arg, int minor_arg, int rev_arg)
      : major(major_arg), minor(minor_arg), revision(rev_arg) {}

  Version(const std::string& ver_str) {
    const std::regex version_regex("v([0-9]*)\\.([0-9]*)\\.([0-9]*)");
    std::smatch version_match;

    if (std::regex_match(ver_str, version_match, version_regex)) {
      major = std::atoi(version_match[1].str().c_str());
      minor = std::atoi(version_match[2].str().c_str());
      revision = std::atoi(version_match[3].str().c_str());
    }
  }

  bool operator<(const Version& rhs) const {
    return (major < rhs.major) ||
           (major == rhs.major &&
            (minor < rhs.minor ||
             (minor == rhs.minor && revision < rhs.revision)));
  }
};

std::ostream& operator<<(std::ostream& out, const Version& ver) {
  return out << "v" << ver.major << "." << ver.minor << "." << ver.revision;
}

constexpr const Version kWizardVersion = Version(0, 1, 0);

#endif /* end of include guard: VERSION_H_C757E53C */
