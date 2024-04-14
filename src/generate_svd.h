//
//  SVD
//
//  Created by Victoria Koreshkova on 29.03.2024.
//
#include <iostream>
#include <Eigen/Dense>
#include "ctime"
#include "tuple"
#include <random>
#include <cassert>
#include <random>
#include <iterator>
#include <algorithm>

//rows - число строк
//cols - число столбцов
//p = min(rows, cols)
//generate_function(int n) - функция, генерирующая сингулярные значения. Должна генерировать n чисел.
//sigmas - массив сингулярных значений
//U - матрица U
//V - матрица V
//generate(int nonzeros) - генерация SVD разложения, где спектр матрицы содержит n ненулевых значений 

template <typename T,int M = Eigen::Dynamic,  int N = Eigen::Dynamic>
class SVDGenerator
{
    private:

    using MatrixUType = Eigen::Matrix<T, M, M>;
    using MatrixVType = Eigen::Matrix<T, N, N>;
    using MatrixSType = Eigen::Matrix<T, M, N>;
    using DynamicMatrix = Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic>;
    using SingValVector =  std::vector<T>;

    bool generatedFLG = false;
    MatrixUType U;
    MatrixVType V;
    MatrixSType S;
    SingValVector sigmas;
    std::default_random_engine RNG;
    std::uniform_real_distribution<T> distribution;
    bool includeBoundaries;
    int rows;
    int cols;
    int p;
    
    void set_sing_vals(const SingValVector& sigmas1){

        assert(p == sigmas1.size());

        sigmas.clear();
        for(int i = 0; i < p; ++i)
        {
            sigmas.push_back(sigmas1[i]);
        }
        std::sort(sigmas.begin(), sigmas.end(), std::greater<T>());
    }

    SingValVector gen_rand_nums(int n){
        SingValVector tmp(n);
        for (int i = 0; i < n; ++i)
            tmp[i] = distribution(RNG);
        if (includeBoundaries) {
            tmp[0]=distribution.a();
            tmp[1]=distribution.b();
        }
        return tmp;
    }

    public:

    SVDGenerator(int rows1, int cols1, const std::default_random_engine RNG_src, const std::uniform_real_distribution<T> dist_src, bool includeBoundaries=false)
    {
        assert(rows1 > 0);
        assert(cols1 > 0);

        rows = rows1;
        cols = cols1;
        p = std::min(rows, cols);

        U = MatrixUType::Zero(rows, rows);
        V = MatrixVType::Zero(cols, cols);
        S = MatrixSType::Zero(rows, cols);

        RNG = RNG_src;
        distribution = dist_src; 
        SingValVector sigmas1 = SingValVector(p);
        std::fill(sigmas1.begin(), sigmas1.end(), T(0));
        set_sing_vals(sigmas1);
    }

    MatrixUType MatrixU()
    {
        if (!generatedFLG)
            generate(p);
        return U;
    }
    
    MatrixVType MatrixV()
    {
        if (!generatedFLG)
            generate(p);
        return V;
    }

    MatrixSType MatrixS()
    {
        if (!generatedFLG)
            generate(p);
        return S;
    }   

    void generate(int nonzeros)
    {
        assert(nonzeros <= p);
        generatedFLG = true;
        std::fill(sigmas.begin(), sigmas.end(), T(0));
        SingValVector nonzero_sigmas = gen_rand_nums(nonzeros);
        std::copy(nonzero_sigmas.begin(), nonzero_sigmas.end(), sigmas.begin());
        std::sort(sigmas.begin(), sigmas.end(), std::greater<T>());
        //U,V-ортогональные матрицы, SIGMA - диагональная матрица
        
        /*Создается две случайные матрицы нужных размеров - T1 и T2,элементы - случайные числа от 0.1 до 10. QR разложение раскладывает матрицу на произведение двух: ортогональной Q и верхнедиагональной R
        С помощью этого разложения случайная матрица превращается в ортогональную и далее эта
        ортогональная матрица Q используется как U или V */
        
        DynamicMatrix T_1(rows, rows), T_2(cols, cols), Q_1(rows,rows), Q_2(cols,cols), R_1(rows,rows), R_2(cols,cols);
        //Сингулярные значения нумеруются в порядке убывания
        //Тут на всякий случай сортируем массив сингулярных чисел, чтобы элменты шли в порядке убывания l1 >= l2 >=.....>= lk >= 0

        S.setZero();
        
        for(int i = 0; i < p; i++)
            S(i,i) = sigmas[i];
            
        
        //Заполнение матриц T_1 и T_2 случайными элементами от 0.1 до 10
        T HI = static_cast<T>(10);
        T LO = static_cast<T>(0.1);
        T range= HI-LO;
        
        T_1 = MatrixUType::Random(rows,rows);
        T_1 = (T_1 + MatrixUType::Constant(rows,rows, static_cast<T>(1)))*range/2;
        T_1 = (T_1 + MatrixUType::Constant(rows, rows, LO));
        
        T_2 = MatrixVType::Random(cols,cols);
        T_2 = (T_2 + MatrixVType::Constant(cols, cols, static_cast<T>(1)))*range/2;
        T_2 = (T_2 + MatrixVType::Constant(cols, cols, LO));
        
        
        //QR_разложение матриц T1 и T2
        Q_1 = (Eigen::FullPivHouseholderQR<Eigen::Matrix<T, N, M>>(T_1)).matrixQ();
        Q_2 = (Eigen::FullPivHouseholderQR<Eigen::Matrix<T, N, M>>(T_2)).matrixQ();
        U = Q_1;
        V = Q_2.transpose();

    }
};