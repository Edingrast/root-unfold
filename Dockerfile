FROM mambaorg/micromamba:2.9.0

USER root

RUN micromamba install -y -n base \
    -c conda-forge \
    root=6.40.04 \
    cmake \
    git \
    make \
    gcc \
    gxx \
    binutils \
    python \
    && micromamba clean --all --yes

ENV PATH=/opt/conda/bin:$PATH

# Build RooUnfold from source
WORKDIR /tmp

RUN git clone --depth 1 --recurse-submodules https://gitlab.cern.ch/RooUnfold/RooUnfold.git \
    && cd RooUnfold \
    && sed -i 's/ RooFitJSONInterface//' CMakeLists.txt \
    && python3 -c "from pathlib import Path; p=Path('src/RooUnfoldTH1Helpers.cxx'); s=p.read_text(); marker='template TH1 *createHist<TH1, TH1>(const TVectorD &vec, const TVectorD &errvec'; insert='template TH1 *createHist<TH1, TH1>(const TVectorD &vec, const char *name, const char *title, const std::vector<Variable<TH1>> &x, bool overflow);\\n'; assert marker in s; p.write_text(s.replace(marker, insert + marker))" \
    && mkdir build \
    && cd build \
    && cmake .. -DCMAKE_CXX_STANDARD=20 \
    && make -j2 \
    && cp libRooUnfold.so /opt/conda/lib/ \
    && cp libRooUnfold_rdict.pcm /opt/conda/lib/ \
    && cp ../src/*.h /opt/conda/include/ \
    && cp ../src/*.tpp /opt/conda/include/ \
    && mkdir -p /opt/conda/include/RooUnfold \
    && cp -r ../src/RooUnfold/TUnfold /opt/conda/include/RooUnfold/ \
    && cd / \
    && rm -rf /tmp/RooUnfold

WORKDIR /work

CMD ["/bin/bash"]