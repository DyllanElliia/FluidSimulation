# FluidSimulation

## 公式推导

### 流体方程推导

$$
m\vec{a}=\vec{F}
$$

$$
m = \rho\times V
$$

>   在SPH中，我们将$V$视为单位体积 $ \Rightarrow m=\rho$

$$
\rho\vec{a}=\vec{F}
$$

$$
\vec{F}=\vec{F}_{external}+\vec{F}_{pressure}+\vec{F}_{viscosity}
$$

>   $\vec{F}_{external}$：外力，这里指重力；
>
>   $\Rightarrow \vec{F}_{external}=\rho\vec{g}$
>
>   $\vec{F}_{pressure}$：流体内压力差产生的压强，数值上等于压力场的梯度，但压力的方向是与压力场的梯度方向相反；
>
>   $\Rightarrow\vec{F}_{pressure}=-\nabla p$
>
>   $\vec{F}_{viscosity}$：流体间由于速度差产生的类似于剪切力的作用力，与粘度（viscosity）系数$\mu$和速度差有关；
>
>   $\Rightarrow\vec{F}_{viscosity}=\mu\nabla^2{\vec{v}}$

$$
Substituted: \rho\vec{a}=\rho\vec{g}-\nabla{p}+\mu\nabla^2{\vec{v}}
$$

$$
Navier-Stokes equation: \rho\frac{\partial{\vec{V}}}{\partial{t}}=\rho\vec{g}-\nabla{p}+\mu\nabla^2{\vec{V}}
$$

$$
Simplified:\vec{a}=\vec{g}-\frac{\nabla{p}}{\rho}+\frac{\mu\nabla^2{\vec{v}}}{\rho}
$$

### SPH推导

$$
\vec{a}(\vec{r_i})=\vec{g}-\frac{\nabla{p}(\vec{r_i})}{\rho(\vec{r_i})}+\frac{\mu\nabla^2{\vec{v}(\vec{r_i})}}{\rho(\vec{r_i})}
$$

>   $\vec{r_i}$：流体中位置为$r_i$的点；
>
>   $\rho(\vec{r_i})$：此处的密度；
>
>   $p(\vec{r_i})$：此处的压力；
>
>   $\vec{v}(\vec{r_i})$：此处的速度；

$\vec{r_i}$在半径h范围内有多个粒子，分别标号为$\vec{r_1}、\vec{r_2}、\vec{r_3}...\vec{r_n}$；

对某个属性 A 的值的累加有累加公式为：
$$
A(\vec{r_i})=\displaystyle \sum^{n}_{j = 1}A_j\frac{m_j}{\rho_j}W(\vec{r_i}-\vec{r_j},h)
$$

>   W：光滑核函数，连点位置间的距离为第一个输入参数，光滑核半径h为第二个输入参数；
>
>   光滑核函数是一个偶函数，W(-r)=W(r)；
>
>   其中$\int_{\Omega}{W(r)dr}=1$；

这里，推荐一篇[知乎文章](https://zhuanlan.zhihu.com/p/26520812?from_voters_page=true)，可以更好地理解光滑核函数在SPH中的概念与意义；

#### 密度推导
$$
\rho(\vec{r_i})=\sum^{n}_{j = 1}\rho_j\frac{m_j}{\rho_j}W(\vec{r_i}-\vec{r_j},h)=\sum^{n}_{j = 1}m_jW(\vec{r_i}-\vec{r_j},h)
$$

密度计算使用的光滑核函数称为Poly6函数：
$$
W_{Poly6}(\vec{r},h)=\bigg\{{K_{poly6}(h^2-|\vec{r}|^2)^3,0\leq r\leq h\atop 0,otherwise}
$$

>   with $r=|\vec{r}|$

对于2D情况，极坐标积分：
$$
\int_{\Omega}{W(r)dr}=1\Rightarrow K_{poly6}=1/\int^{2\pi}_{0}{\int^{h}_{0}{(h^2-r^2)rdrd\theta}}=\frac{4}{\pi h^8}
$$
对于3D情况，球坐标积分：
$$
K_{poly6}=1/\int^{\pi}_{0}{\int^{2\pi}_{0}{\int^{h}_{0}{(h^2-r^2)r^2sin(\phi)drd\phi d\theta}}}=\frac{315}{64\pi h^9}
$$

该项目的流体属于对3D流体的模拟，因此讨论3D情况下的密度计算公式：
$$
\rho(\vec{r_i})=\frac{315}{64\pi h^9}\sum^{n}_{j = 1}m_j(h^2-|\vec{r_i}-\vec{r_j}|^2)^3
$$
若仅模拟单一流体，则所有粒子的质量相同都是m：
$$
\rho(\vec{r_i})=m\frac{315}{64\pi h^9}\sum^{n}_{j = 1}(h^2-|\vec{r_i}-\vec{r_j}|^2)^3
$$

>   若有多种流体该怎么处理呢？求和保留$m_j$计算累加？还是直接认定m为$r_i$的m？

#### 压力推导

$$
\vec{F}^{pressure}_i=-\nabla p(\vec{r_i})=-\sum^{n}_{j = 1}p_j\frac{m_j}{\rho_j}\nabla W(\vec{r_i}-\vec{r_j},h)
$$

>   位于不同压强区的两个粒子之间的作用力不相等；
>
>   所以计算中一般取双方粒子压强的算术平均代替单个粒子的压强；

$$
Subsitituted:\vec{F}^{pressure}_i=-\nabla p(\vec{r_i})=-\sum^{n}_{j = 1}\frac{m_j(p_i+p_j)}{2\rho_j}\nabla W(\vec{r_i}-\vec{r_j},h)
$$

对于单个粒子产生的压强p，可以使用理想气态状态方程计算：
$$
p = K(\rho-\rho_0)
$$

>   $\rho_0$：流体的静态密度；
>
>   K：和流体相关的常数，且只和温度相关；

压力采用Spiky函数作为光滑核函数：
$$
W_{Spiky}(\vec{r},h)=\Bigg\{{K_{Spiky}(h-r)^3,1\leq r\leq h \atop 0.otherwise}
$$

>   with $r=|\vec{r}|$

同样，仅求3D情况：
$$
K_{Spiky}=1/\int^{\pi}_{0}{\int^{2\pi}_{0}{\int^{h}_{0}{(h-r)r^2sin(\phi)drd\phi d\theta}}}=\frac{15}{\pi h^6}
$$

$$
Substituted:\nabla W_{Spiky}(\vec{r},h)=\frac{15}{\pi h^6}\nabla (h-r)^3=-\vec{r}\frac{45}{\pi h^6|\vec{r}|}(h-r)^2
$$

同样假设仅对一种流体进行分析：
$$
-\frac{\nabla p(\vec{r_i})}{\rho_i}=m\frac{45}{2\rho_i\pi h^6}\sum^{n}_{i=1}\bigg[\frac{(p_i+p_j)(\vec{r_i}-\vec{r_j})}{\rho_j |\vec{r_i}-\vec{r_j}|}(h-|\vec{r_i}-\vec{r_j}|)^2\bigg]
$$

#### 粘度推导

$$
\vec{F}^{viscosity}_i=\mu\nabla^2u(\vec{r_i})=\mu\sum^{n}_{j = 1}\vec{u_j}\frac{m_j}{\rho_j}\nabla^2W(\vec{r_i}-\vec{r_j},h)
$$

和压力一样，同样存在作用力不相等问题：
$$
Subsitituted:\vec{F}^{viscosity}_i=\mu\nabla^2u(\vec{r_i})=\mu\sum^{n}_{j = 1}\frac{m_j(\vec{u_j}-\vec{u_i})}{2\rho_j}\nabla^2W(\vec{r_i}-\vec{r_j},h)
$$
粘度采用visicosity作为光滑核函数：
$$
W_{Viscosity}(\vec{r},h)=\Bigg\{{K_{Viscosity}\bigg(-\frac{r^3}{2h^3}+\frac{r^2}{h^2}+\frac{h}{2r}-1\bigg),1\leq r\leq h \atop 0.otherwise}
$$

>   with $r=|\vec{r}|$

同样，在3D情况下：
$$
K_{Viscosity}=1/\int^{\pi}_{0}{\int^{2\pi}_{0}{\int^{h}_{0}{\bigg(-\frac{r^3}{2h^3}+\frac{r^2}{h^2}+\frac{h}{2r}-1\bigg)r^2sin(\phi)drd\phi d\theta}}}=\frac{15}{2\pi h^3}
$$

$$
Substituted:\nabla^2W_{Viscosity}(\vec{r},h)=\frac{15}{2\pi h^2}\nabla^2\bigg(-\frac{r^3}{2h^3}+\frac{r^2}{h^2}+\frac{h}{2r}-1\bigg)=\frac{45}{\pi h^6}(h-r)
$$

$$
Subsitituted:\frac{\mu\nabla^2{\vec{v}(\vec{r_i})}}{\rho(\vec{r_i})}=m\mu\frac{45}{\rho_i\pi h^6}\sum^n_{j=1}\frac{\vec{u_j}-\vec{u_i}}{\rho_j}(h-|\vec{r_i}-\vec{r_j}|)
$$

#### FINAL

$$
a(\vec{r_i})=\vec{g}+m\frac{45}{\rho_i\pi h^6}\sum^{n}_{i=1}\bigg[\frac{(p_i+p_j)(\vec{r_i}-\vec{r_j})}{2\rho_j |\vec{r_i}-\vec{r_j}|}(h-|\vec{r_i}-\vec{r_j}|)^2\bigg]+m\mu\frac{45}{\rho_i\pi h^6}\sum^n_{j=1}\frac{\vec{u_j}-\vec{u_i}}{\rho_j}(h-|\vec{r_i}-\vec{r_j}|)
$$

其中，$\rho_i$见公式 (15)。

