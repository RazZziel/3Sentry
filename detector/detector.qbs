import qbs
import qbs.probes as Probes

CppApplication {
    type: "application" // To suppress bundle generation on Mac
    consoleApplication: true
    files: ["camshifting.cpp",
            "trackface.cpp",
            "camshifting.h",
            "trackface.h"]

    Group {     // Properties for the produced executable
        fileTagsFilter: product.type
        qbs.install: true
    }

    Probes.PkgConfigProbe {
        id: pkgConfig
        name: "opencv"
    }
}

