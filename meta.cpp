#include <iostream>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <exiv2/exiv2.hpp>

namespace fs = std::filesystem;

std::string format_time(std::filesystem::file_time_type file_time) {
    std::time_t cftime = decltype(file_time)::clock::to_time_t(file_time);
    std::tm *tm = std::localtime(&cftime);
    std::ostringstream oss;
    oss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void generate_report(const std::string& directory_path, const std::string& report_file_path) {
    std::ofstream report(report_file_path);

    if (!report.is_open()) {
        std::cerr << "Failed to open report file for writing: " << report_file_path << std::endl;
        return;
    }

    for (const auto& entry : fs::directory_iterator(directory_path)) {
        if (entry.is_regular_file()) {
            auto file_path = entry.path();
            auto file_size = fs::file_size(file_path);
            auto last_write_time = fs::last_write_time(file_path);

            report << "File: " << file_path << '\n'
                   << "Size: " << file_size << " bytes\n"
                   << "Last Modified: " << format_time(last_write_time) << '\n';

            // Extract EXIF data if it's an image
            try {
                Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(file_path.string());
                image->readMetadata();
                Exiv2::ExifData &exifData = image->exifData();

                if (exifData.empty()) {
                    report << "No EXIF data found in the file.\n";
                } else {
                    // Print GPS data if available
                    if (exifData.findKey(Exiv2::ExifKey("Exif.GPSInfo.GPSLatitude")) != exifData.end() &&
                        exifData.findKey(Exiv2::ExifKey("Exif.GPSInfo.GPSLongitude")) != exifData.end()) {
                        float latitude = exifData["Exif.GPSInfo.GPSLatitude"];
                        float longitude = exifData["Exif.GPSInfo.GPSLongitude"];
                        report << "GPS Latitude: " << latitude << "\n";
                        report << "GPS Longitude: " << longitude << "\n";
                    } else {
                        report << "No GPS data found.\n";
                    }
                }
            } catch (const Exiv2::Error& e) {
                report << "Error reading EXIF data: " << e.what() << '\n';
            }

            report << "------------------------\n";
        }
    }

    report.close();
    std::cout << "Report generated at: " << report_file_path << std::endl;
}

int main() {
    std::string directory_path = "path_to_your_directory";
    std::string report_file_path = "report.txt";

    generate_report(directory_path, report_file_path);

    return 0;
}
