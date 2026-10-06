buildscript {
    dependencies {
        classpath(libs.kotlin.gradle)
    }
}

plugins {
    alias(libs.plugins.android.library) apply false
}
