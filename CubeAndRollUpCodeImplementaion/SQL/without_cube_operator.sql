EXPLAIN ANALYZE
-- Revenue by All Attributes
SELECT
    NULL AS Region,
    NULL AS Store,
    NULL AS Product,
    NULL AS Category,
    NULL AS Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
UNION ALL
-- Revenue by Region
SELECT
    Region,
    NULL AS Store,
    NULL AS Product,
    NULL AS Category,
    NULL AS Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    Region
UNION ALL
-- Revenue by Store
SELECT
    NULL AS Region,
    Store,
    NULL AS Product,
    NULL AS Category,
    NULL AS Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    Store
UNION ALL
-- Revenue by Product
SELECT
    NULL AS Region,
    NULL AS Store,
    Product,
    NULL AS Category,
    NULL AS Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    Product
UNION ALL
-- Revenue by Category
SELECT
    NULL AS Region,
    NULL AS Store,
    NULL AS Product,
    Category,
    NULL AS Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    Category
UNION ALL
-- Revenue by Quarter
SELECT
    NULL AS Region,
    NULL AS Store,
    NULL AS Product,
    NULL AS Category,
    Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    Quarter
UNION ALL
-- Revenue by Region and Store
SELECT
    Region,
    Store,
    NULL AS Product,
    NULL AS Category,
    NULL AS Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    Region,
    Store
UNION ALL
-- Revenue by Region and Product
SELECT
    Region,
    NULL AS Store,
    Product,
    NULL AS Category,
    NULL AS Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    Region,
    Product
UNION ALL
-- Revenue by Region and Category
SELECT
    Region,
    NULL AS Store,
    NULL AS Product,
    Category,
    NULL AS Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    Region,
    Category
UNION ALL
-- Revenue by Region and Quarter
SELECT
    Region,
    NULL AS Store,
    NULL AS Product,
    NULL AS Category,
    Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    Region,
    Quarter
UNION ALL
-- Revenue by Store and Product
SELECT
    NULL AS Region,
    Store,
    Product,
    NULL AS Category,
    NULL AS Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    Store,
    Product
UNION ALL
-- Revenue by Store and Category
SELECT
    NULL AS Region,
    Store,
    NULL AS Product,
    Category,
    NULL AS Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    Store,
    Category
UNION ALL
-- Revenue by Store and Quarter
SELECT
    NULL AS Region,
    Store,
    NULL AS Product,
    NULL AS Category,
    Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    Store,
    Quarter
UNION ALL
-- Revenue by Product and Category
SELECT
    NULL AS Region,
    NULL AS Store,
    Product,
    Category,
    NULL AS Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    Product,
    Category
UNION ALL
-- Revenue by Product and Quarter
SELECT
    NULL AS Region,
    NULL AS Store,
    Product,
    NULL AS Category,
    Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    Product,
    Quarter
UNION ALL
-- Revenue by Category and Quarter
SELECT
    NULL AS Region,
    NULL AS Store,
    NULL AS Product,
    Category,
    Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    Category,
    Quarter
UNION ALL
-- Revenue by Region, Store, and Product
SELECT
    Region,
    Store,
    Product,
    NULL AS Category,
    NULL AS Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    Region,
    Store,
    Product
UNION ALL
-- Revenue by Region, Store, and Category
SELECT
    Region,
    Store,
    NULL AS Product,
    Category,
    NULL AS Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    Region,
    Store,
    Category
UNION ALL
-- Revenue by Region, Store, and Quarter
SELECT
    Region,
    Store,
    NULL AS Product,
    NULL AS Category,
    Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    Region,
    Store,
    Quarter
UNION ALL
-- Revenue by Region, Product, and Category
SELECT
    Region,
    NULL AS Store,
    Product,
    Category,
    NULL AS Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    Region,
    Product,
    Category
UNION ALL
-- Revenue by Region, Product, and Quarter
SELECT
    Region,
    NULL AS Store,
    Product,
    NULL AS Category,
    Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    Region,
    Product,
    Quarter
UNION ALL
-- Revenue by Region, Category, and Quarter
SELECT
    Region,
    NULL AS Store,
    NULL AS Product,
    Category,
    Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    Region,
    Category,
    Quarter
UNION ALL
-- Revenue by Store, Product, and Category
SELECT
    NULL AS Region,
    Store,
    Product,
    Category,
    NULL AS Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    Store,
    Product,
    Category
UNION ALL
-- Revenue by Store, Product, and Quarter
SELECT
    NULL AS Region,
    Store,
    Product,
    NULL AS Category,
    Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    Store,
    Product,
    Quarter
UNION ALL
-- Revenue by Store, Category, and Quarter
SELECT
    NULL AS Region,
    Store,
    NULL AS Product,
    Category,
    Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    Store,
    Category,
    Quarter
UNION ALL
-- Revenue by Product, Category, and Quarter
SELECT
    NULL AS Region,
    NULL AS Store,
    Product,
    Category,
    Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    Product,
    Category,
    Quarter
UNION ALL
-- Revenue by Region, Store, Product, and Category
SELECT
    Region,
    Store,
    Product,
    Category,
    NULL AS Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    Region,
    Store,
    Product,
    Category
UNION ALL
-- Revenue by Region, Store, Product, and Quarter
SELECT
    Region,
    Store,
    Product,
    NULL AS Category,
    Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    Region,
    Store,
    Product,
    Quarter
UNION ALL
-- Revenue by Region, Store, Category, and Quarter
SELECT
    Region,
    Store,
    NULL AS Product,
    Category,
    Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    Region,
    Store,
    Category,
    Quarter
UNION ALL
-- Revenue by Region, Product, Category, and Quarter
SELECT
    Region,
    NULL AS Store,
    Product,
    Category,
    Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    Region,
    Product,
    Category,
    Quarter
UNION ALL
-- Revenue by Store, Product, Category, and Quarter
SELECT
    NULL AS Region,
    Store,
    Product,
    Category,
    Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    Store,
    Product,
    Category,
    Quarter
UNION ALL
-- Revenue by Region, Store, Product, Category, and Quarter (Grand Total)
SELECT
    Region,
    Store,
    Product,
    Category,
    Quarter,
    SUM(Revenue) AS SUM_Revenue,
    COUNT(Revenue) AS COUNT_Revenue,
    MIN(Revenue) AS MIN_Revenue,
    MAX(Revenue) AS MAX_Revenue,
    AVG(Revenue) AS AVG_Revenue
FROM
    SalesTable
GROUP BY
    Region,
    Store,
    Product,
    Category,
    Quarter
ORDER BY
    Region,
    Product,
    Category,
    Store,
    Quarter;