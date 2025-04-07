rm -rf /project/build /project/bin
cmake -S src -B build -G Ninja
ninja -C /project/build

mkdir /project/bin
cp /project/build/fastibs /project/bin
cp /project/build/fastibsmapper /project/bin
cp /project/build/KDBIntersect /project/bin

echo "Build completed successfully."

